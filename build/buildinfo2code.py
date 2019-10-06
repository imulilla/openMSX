from cpu import getCPU, X86, X86_64
from makeutils import extractMakeVariables, parseBool
from outpututils import rewriteIfChanged

from os.path import dirname, join as joinpath
import sys

def iterBuildInfoHeader(targetPlatform, cpuName, flavour, installShareDir):
	platformVars = extractMakeVariables(
        joinpath(dirname(__file__), 'platform-%s.mk' % targetPlatform),
		dict.fromkeys(
			('COMPILE_FLAGS', 'LINK_FLAGS', 'LDFLAGS', 'TARGET_FLAGS',
				'OPENMSX_TARGET_CPU'),
			''
			)
		)
	setWindowIcon = parseBool(platformVars.get('SET_WINDOW_ICON', 'true'))

	targetCPU = getCPU(cpuName)

	# TODO: Add support for device-specific configuration.
	platformDingux = targetPlatform == 'dingux'
	platformPandora = targetPlatform == 'pandora'
	platformAndroid = targetPlatform == 'android'

	# Defaults.
	have16BPP = True
	have32BPP = True
	minScaleFactor = 1
	maxScaleFactor = 4

	# Platform overrides.
	if platformDingux:
		maxScaleFactor = 1
	elif platformAndroid:
		# At the moment, libsdl android crashes when trying to dynamically change the scale factor
		# TODO: debug why it crashes and then change the maxScaleFactor parameter here
		# so that people with a powerfull enough android device can use a higher scale factor
		have32BPP = False
		minScaleFactor = 2
		maxScaleFactor = 2
	elif platformPandora:
		have32BPP = False
		maxScaleFactor = 3

	yield '// Automatically generated by build process.'
	yield ''
	yield '#ifndef BUILD_INFO_HH'
	yield '#define BUILD_INFO_HH'
	yield ''
	# Use a macro i.s.o. a boolean to prevent compilation errors on inline asm.
	# Assembly doesn't appear to work with MINGW64... TODO: find out why
	yield '#ifdef __MINGW64__'
	yield '#define ASM_X86 0'
	yield '#define ASM_X86 0'
	yield '#define ASM_X86_32 0'
	yield '#define ASM_X86_64 0'
	yield '#else'
	# A compiler will typically only understand the instruction set that it
	# generates code for.
	yield '#define ASM_X86 %d' % (targetCPU is X86 or targetCPU is X86_64)
	yield '#define ASM_X86_32 %d' % (targetCPU is X86)
	yield '#define ASM_X86_64 %d' % (targetCPU is X86_64)
	yield '#endif'
	# Use a macro iso integer because we really need to exclude code sections
	# based on this.
	yield '#define PLATFORM_DINGUX %d' % platformDingux
	yield '#define PLATFORM_ANDROID %d' % platformAndroid
	yield '#define HAVE_16BPP %d' % have16BPP
	yield '#define HAVE_32BPP %d' % have32BPP
	yield '#define MIN_SCALE_FACTOR %d' % minScaleFactor
	yield '#define MAX_SCALE_FACTOR %d' % maxScaleFactor
	yield ''
	yield 'namespace openmsx {'
	yield ''
	# Note: Don't call it "BIG_ENDIAN", because some system header may #define
	#       that.
	yield 'static const bool OPENMSX_BIGENDIAN = %s;' \
		% str(targetCPU.bigEndian).lower()
	yield 'static const bool OPENMSX_UNALIGNED_MEMORY_ACCESS = %s;' \
		% str(targetCPU.unalignedMemoryAccess).lower()
	yield 'static const bool OPENMSX_SET_WINDOW_ICON = %s;' \
		% str(setWindowIcon).lower()
	yield 'static const char* const DATADIR = "%s";' % installShareDir
	yield 'static const char* const BUILD_FLAVOUR = "%s";' % flavour
	yield 'static const char* const TARGET_PLATFORM = "%s";' % targetPlatform
	yield ''
	yield '} // namespace openmsx'
	yield ''
	yield '#endif // BUILD_INFO_HH'

if __name__ == '__main__':
	if len(sys.argv) == 6:
		rewriteIfChanged(sys.argv[1], iterBuildInfoHeader(*sys.argv[2 : ]))
	else:
		print(
			'Usage: python3 buildinfo2code.py CONFIG_HEADER '
			'platform cpu flavour share-install-dir',
			file=sys.stderr
			)
		sys.exit(2)
