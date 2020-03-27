#include "YamahaFDC.hh"
#include "CacheLine.hh"
#include "DriveMultiplexer.hh"
#include "MSXException.hh"
#include "Rom.hh"
#include "WD2793.hh"
#include "serialize.hh"

// This is derived by disassembly of the Yamaha FD-03 diskrom
//   https://sourceforge.net/p/msxsyssrc/git/ci/master/tree/diskdrvs/fd-03/
//
// FDD interface:
// 7FC0   I/O            FDC STATUS/COMMAND
// 7FC1   I/O            FDC TRACK REGISTER
// 7FC2   I/O            FDC SECTOR REGISTER
// 7FC3   I/O            FDC DATA REGISTER
// 7FE0   O     bit 0    SELECT DRIVE A              "1" ON
//        I     bit 0    READY DRIVE A               "0" READY, "1" NOT READY
//        O     bit 1    SELECT DRIVE B              "1" ON
//        I     bit 1    READY DRIVE B               "0" READY, "1" NOT READY
//        O     bit 2    MOTOR                       "1" ON
//        I     bit 2    DISK CHANGE DRIVE A         "1" CHANGED
//        O     bit 3    UNKNOWN FUNCTION
//        I     bit 3    DISK CHANGE DRIVE B         "1" CHANGED
//        I     bit 6    FDC DATA REQUEST            "1" REQUEST
//        I     bit 7    FDC INTERRUPT REQUEST       "1" REQUEST
//
// 7FF0   O              RESET DISK CHANGE DRIVE A
//        I              RESET DISK CHANGE DRIVE B

namespace openmsx {

static const int DRIVE_A_SELECT = 0x01;
static const int DRIVE_B_SELECT = 0x02;
static const int DRIVE_A_NOT_READY = 0x01;
static const int DRIVE_B_NOT_READY = 0x02;
static const int DISK_A_CHANGED = 0x04;
static const int DISK_B_CHANGED = 0x08;
static const int MOTOR_ON = 0x04;
static const int DATA_REQUEST  = 0x40;
static const int INTR_REQUEST  = 0x80;


YamahaFDC::YamahaFDC(const DeviceConfig& config)
	: WD2793BasedFDC(config, "", true, DiskDrive::TrackMode::YAMAHA_FD_03)
{
	if ((rom->getSize() != 0x4000) && (rom->getSize() != 0x8000)) {
		throw MSXException("YamahaFDC ROM size must be 16kB or 32kB.");
	}
	reset(getCurrentTime());
}

void YamahaFDC::reset(EmuTime::param time)
{
	WD2793BasedFDC::reset(time);
	writeMem(0x7FE0, 0x00, time);
}

byte YamahaFDC::readMem(word address, EmuTime::param time)
{
	byte value;
	switch (address & 0x3FFF) {
	case 0x3FC0:
		value = controller.getStatusReg(time);
		break;
	case 0x3FC1:
		value = controller.getTrackReg(time);
		break;
	case 0x3FC2:
		value = controller.getSectorReg(time);
		break;
	case 0x3FC3:
		value = controller.getDataReg(time);
		break;
	case 0x3FE0:
		value = 0;
		if (!multiplexer.isDiskInserted(DriveMultiplexer::DRIVE_A)) value |= DRIVE_A_NOT_READY;
		if (!multiplexer.isDiskInserted(DriveMultiplexer::DRIVE_B)) value |= DRIVE_B_NOT_READY;
		// note: peekDiskChanged() instead of diskChanged() because we don't want an implicit reset
		if (multiplexer.peekDiskChanged(DriveMultiplexer::DRIVE_A)) value |= DISK_A_CHANGED;
		if (multiplexer.peekDiskChanged(DriveMultiplexer::DRIVE_B)) value |= DISK_B_CHANGED;
		if (controller.getIRQ(time))  value |= INTR_REQUEST;
		if (controller.getDTRQ(time)) value |= DATA_REQUEST;
		break;
	case 0x3FF0:
		multiplexer.diskChanged(DriveMultiplexer::DRIVE_B);
		value = peekMem(address, time);
		break;
	default:
		value = peekMem(address, time);
		break;
	}
	return value;
}

byte YamahaFDC::peekMem(word address, EmuTime::param time) const
{
	byte value;
	switch (address & 0x3FFF) {
	case 0x3FC0:
		value = controller.peekStatusReg(time);
		break;
	case 0x3FC1:
		value = controller.peekTrackReg(time);
		break;
	case 0x3FC2:
		value = controller.peekSectorReg(time);
		break;
	case 0x3FC3:
		value = controller.peekDataReg(time);
		break;
	case 0x3FE0:
		value = 0;
		if (!multiplexer.isDiskInserted(DriveMultiplexer::DRIVE_A)) value |= DRIVE_A_NOT_READY;
		if (!multiplexer.isDiskInserted(DriveMultiplexer::DRIVE_B)) value |= DRIVE_B_NOT_READY;
		if (multiplexer.peekDiskChanged(DriveMultiplexer::DRIVE_A)) value |= DISK_A_CHANGED;
		if (multiplexer.peekDiskChanged(DriveMultiplexer::DRIVE_B)) value |= DISK_B_CHANGED;
		if (controller.peekIRQ(time))  value |= INTR_REQUEST;
		if (controller.peekDTRQ(time)) value |= DATA_REQUEST;
		break;
	case 0x3FF0:
		// don't clear on peek
		[[fallthrough]];
	default:
		if (unsigned(address - 0x4000) < rom->getSize()) {
			value = (*rom)[address - 0x4000];
		} else {
			value = 255;
		}
		break;
	}
	return value;
}

const byte* YamahaFDC::getReadCacheLine(word start) const
{
	if ((start & 0x3FFF & CacheLine::HIGH) == (0x3FC0 & CacheLine::HIGH)) {
		// FDC at 0x7FC0-0x7FFF  or  0xBFC0-0xBFFF
		return nullptr;
	} else if (unsigned(start - 0x4000) < rom->getSize()) {
		return &(*rom)[start - 0x4000];
	} else {
		return unmappedRead;
	}
}

void YamahaFDC::writeMem(word address, byte value, EmuTime::param time)
{
	switch (address & 0x3fff) {
	case 0x3FC0:
		controller.setCommandReg(value, time);
		break;
	case 0x3FC1:
		controller.setTrackReg(value, time);
		break;
	case 0x3FC2:
		controller.setSectorReg(value, time);
		break;
	case 0x3FC3:
		controller.setDataReg(value, time);
		break;
	case 0x3FE0:
		DriveMultiplexer::DriveNum drive;
		switch (value & (DRIVE_A_SELECT | DRIVE_B_SELECT)) {
		case DRIVE_A_SELECT:
			drive = DriveMultiplexer::DRIVE_A;
			break;
		case DRIVE_B_SELECT:
			drive = DriveMultiplexer::DRIVE_B;
			break;
		default:
			// No drive selected or two drives at same time
			// The motor is enabled for all drives at the same time, so
			// in a real machine you must take care to do not select more
			// than one drive at the same time (you could get data
			// collision).
			drive = DriveMultiplexer::NO_DRIVE;
		}
		multiplexer.selectDrive(drive, time);
		multiplexer.setSide(false);
		multiplexer.setMotor((value & MOTOR_ON) != 0, time);
		break;
	case 0x3FF0:
		multiplexer.diskChanged(DriveMultiplexer::DRIVE_A);
		break;
	}
}

byte* YamahaFDC::getWriteCacheLine(word address) const
{
	if ((address & 0x3FFF & CacheLine::HIGH) == (0x3FC0 & CacheLine::HIGH)) {
		// FDC at 0x7FC0-0x7FFF  or  0xBFC0-0xBFFF
		return nullptr;
	} else {
		return unmappedWrite;
	}
}

template<typename Archive>
void YamahaFDC::serialize(Archive& ar, unsigned /*version*/)
{
	ar.template serializeBase<WD2793BasedFDC>(*this);
}
INSTANTIATE_SERIALIZE_METHODS(YamahaFDC);
REGISTER_MSXDEVICE(YamahaFDC, "YamahaFDC");

} // namespace openmsx
