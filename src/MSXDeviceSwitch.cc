// $Id$

#include "MSXDeviceSwitch.hh"
#include "MSXSwitchedDevice.hh"
#include "MSXCPUInterface.hh"
#include "MSXMotherBoard.hh"
#include "MSXException.hh"
#include "StringOp.hh"
#include <cassert>

namespace openmsx {

MSXDeviceSwitch::MSXDeviceSwitch(MSXMotherBoard& motherBoard,
                                 const XMLElement& config)
	: MSXDevice(motherBoard, config)
{
	for (int i = 0; i < 256; ++i) {
		devices[i] = NULL;
	}
	count = 0;
	selected = 0;
}

MSXDeviceSwitch::~MSXDeviceSwitch()
{
	for (int i = 0; i < 256; i++) {
		// all devices must be unregistered
		assert(devices[i] == NULL);
	}
	assert(count == 0);
}


void MSXDeviceSwitch::registerDevice(byte id, MSXSwitchedDevice* device)
{
	//PRT_DEBUG("Switch: register device with id " << (int)id);
	if (devices[id]) {
		// TODO implement multiplexing
		throw MSXException("Already have a switched device with id " +
		                   StringOp::toString(int(id)));
	}
	devices[id] = device;
	if (count == 0) {
		MSXCPUInterface& interface = getMotherBoard().getCPUInterface();
		for (byte port = 0x40; port < 0x50; ++port) {
			interface.register_IO_In (port, this);
			interface.register_IO_Out(port, this);
		}
	}
	++count;
}

void MSXDeviceSwitch::unregisterDevice(byte id)
{
	--count;
	if (count == 0) {
		MSXCPUInterface& interface = getMotherBoard().getCPUInterface();
		for (byte port = 0x40; port < 0x50; ++port) {
			interface.unregister_IO_Out(port, this);
			interface.unregister_IO_In (port, this);
		}
	}
	assert(devices[id] != NULL);
	devices[id] = NULL;
}


void MSXDeviceSwitch::reset(const EmuTime& time)
{
	selected = 0;
}

byte MSXDeviceSwitch::readIO(word port, const EmuTime& time)
{
	if (devices[selected]) {
		//PRT_DEBUG("Switch read device " << (int)selected << " port " << (int)port);
		return devices[selected]->readIO(port, time);
	} else {
		//PRT_DEBUG("Switch read no device");
		return 0xFF;
	}
}

byte MSXDeviceSwitch::peekIO(word port, const EmuTime& time) const
{
	if (devices[selected]) {
		return devices[selected]->peekIO(port, time);
	} else {
		return 0xFF;
	}
}

void MSXDeviceSwitch::writeIO(word port, byte value, const EmuTime& time)
{
	port &= 0x0F;
	if (port == 0x00) {
		selected = value;
		PRT_DEBUG("Switch " << int(selected));
	} else if (devices[selected]) {
		//PRT_DEBUG("Switch write device " << (int)selected << " port " << (int)port);
		devices[selected]->writeIO(port, value, time);
	} else {
		//ignore
	}
}

} // namespace openmsx
