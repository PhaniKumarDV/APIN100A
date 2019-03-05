# vim: set ts=4 sw=4 noexpandtab smarttab autoindent:
from struct import unpack
from optparse import OptionParser
from math import log
import sys, re

#
# Parser self-registration boilerplate
#

parsers = {}

class Parser(type):
	def __new__(cls, name, bases, attrs):
		if 'id' not in attrs:
			raise 'Must provide the ID of the log message this parser handles.'
		return super(Parser, cls).__new__(cls, name, bases, attrs)

	def __init__(self, name, bases, attrs):
		super(Parser, self).__init__(name, bases, attrs)
		parsers[attrs['id']] = self

#
# NPA-related pretty-printing
#
npa_client_names     = {}
npa_client_resources = {}
npa_resources        = {}

def npa_lookup(dictionary, handle):
	if handle in dictionary:
		return dictionary[handle]
	else:
		formatted_handle = '0x%0.8x' % handle
		print '  ***** WARNING ***** failed to match npa handle', formatted_handle
		return formatted_handle

def npa_not_loaded(handle):
	global npa_client_name_lookup
	global npa_client_resource_lookup
	global npa_resource_lookup

	print '  ***** WARNING ***** no NPA dump loaded.  All NPA name resolutions will fail.'
	print '  ***** WARNING ***** switching to silent NPA lookup failure mode.'
	npa_client_name_lookup     = lambda h: '0x%0.8x' % h
	npa_client_resource_lookup = lambda h: '0x%0.8x' % h
	npa_resource_lookup        = lambda h: '0x%0.8x' % h
	return '0x%0.8x' % handle

npa_client_name_lookup     = npa_not_loaded
npa_client_resource_lookup = npa_not_loaded
npa_resource_lookup        = npa_not_loaded

npa_client_regex   = re.compile(r'[^:]*:\s+npa_client\s+\(name: (?P<name>[^\)]*)\)\s+\(handle: (?P<handle>[^\)]+)\)\s+\(resource: (?P<resource>[^\)]+)\)')
npa_resource_regex = re.compile(r'[^:]*:\s+npa_resource\s+\(name: "(?P<name>[^"]+)"\)\s+\(handle: (?P<handle>[^\)]+)')

def parse_npa_dump(npa_dump_filename):
	global npa_client_name_lookup
	global npa_client_resource_lookup
	global npa_resource_lookup

	npa_dump = open(npa_dump_filename, 'r')
	for line in npa_dump.readlines():
		m = npa_client_regex.match(line)
		if m:
			npa_client_names[int(m.group('handle'), 16)] = m.group('name')
			npa_client_resources[int(m.group('handle'), 16)] = int(m.group('resource'), 16)
			continue
		m = npa_resource_regex.match(line)
		if m:
			npa_resources[int(m.group('handle'), 16)] = m.group('name')
			continue
	npa_dump.close()

	npa_client_name_lookup     = lambda h: npa_lookup(npa_client_names, h)
	npa_resource_lookup        = lambda h: npa_lookup(npa_resources, h)
	def client_resource_lookup(h):
		resource = npa_lookup(npa_client_resources, h)
		lookup_failed = isinstance(resource, str)
		return '<lookup failed>' if lookup_failed else npa_resource_lookup(resource)
	npa_client_resource_lookup = client_resource_lookup

#
# Target-specific pretty-printing
#

# This variable starts as a dictionary of targets.  When command line arguments
# are read, it is overwritten with only the specific dictionary for the target
# selected.
target_data = {
	'8660' : {
		'resources' : {
			0	: '"Notifications"',
			1	: '"Request Invalidate"',
			2	: '"Timed Trigger"',
			3	: '"RPM Control"',
			4	: '"Trigger Clear"',
			5	: '"CXO"',
			6	: '"PXO"',
			7	: '"PLL 4"',
			8	: '"Apps Fabric Clock"',
			9	: '"System Fabric Clock"',
			10	: '"MM Fabric Clock"',
			11	: '"Daytona Fabric Clock"',
			12  : '"SFPB Clock"',
			13  : '"CFPB Clock"',
			14  : '"MMFPB Clock"',
			15  : '"SMI Clock"',
			16  : '"EBI1_CLK"',
			17  : '"Apps L2 Cache"',
			18  : '"Apps Fabric Halt"',
			19  : '"Apps Fabric Clock Mode"',
			20  : '"Apps Fabric IOCTL"',
			21  : '"Apps Fabric Arbitration"',
			22  : '"System Fabric Halt"',
			23  : '"System Fabric Clock Mode"',
			24  : '"System Fabric IOCTL"',
			25  : '"System Fabric Arbitration"',
			26  : '"MM Fabric Halt"',
			27  : '"MM Fabric Clock Mode"',
			28  : '"MM Fabric IOCTL"',
			29  : '"MM Fabric Arbitration"',
			30  : '"SMPS0B"',
			31  : '"SMPS1B"',
			32  : '"SMPS2B"',
			33  : '"SMPS3B"',
			34  : '"SMPS4B"',
			35  : '"LDO0B"',
			36  : '"LDO1B"',
			37  : '"LDO2B"',
			38  : '"LDO3B"',
			39  : '"LDO4B"',
			40  : '"LDO5B"',
			41  : '"LDO6B"',
			42  : '"LVS0B"',
			43  : '"LVS1B"',
			44  : '"LVS2B"',
			45  : '"LVS3B"',
			46  : '"MVS"',
			47  : '"SMPS0"',
			48  : '"SMPS1"',
			49  : '"SMPS2"',
			50  : '"SMPS3"',
			51  : '"SMPS4"',
			52  : '"LDO0"',
			53  : '"LDO1"',
			54  : '"LDO2"',
			55  : '"LDO3"',
			56  : '"LDO4"',
			57  : '"LDO5"',
			58  : '"LDO6"',
			59  : '"LDO7"',
			60  : '"LDO8"',
			61  : '"LDO9"',
			62  : '"LDO10"',
			63  : '"LDO11"',
			64  : '"LDO12"',
			65  : '"LDO13"',
			66  : '"LDO14"',
			67  : '"LDO15"',
			68  : '"LDO16"',
			69  : '"LDO17"',
			70  : '"LDO18"',
			71  : '"LDO19"',
			72  : '"LDO20"',
			73  : '"LDO21"',
			74  : '"LDO22"',
			75  : '"LDO23"',
			76  : '"LDO24"',
			77  : '"LDO25"',
			78  : '"LVS0"',
			79  : '"LVS1"',
			80  : '"NCP"',
			81  : '"CXO_BUFFERS"',
			82  : '"USB_OTG_SWITCH"',
			83  : '"HDMI_SWITCH"',
		},
		'masters' : {
			0	: '"APSS"',
			1	: '"MSS"',
			2	: '"LPASS"',
		},
		'mpm_ints' : {
			0	: 'timetick',
			1	: 'touchscreen pen irq / gpio 61',
			2	: 'spare / reserved',
			3	: 'rpm grouped interrupt',
			4	: 'pm8058 sec / gpio 87',
			5	: 'pm8058 usr / gpio 88',
			6	: 'pm8058 mdm / gpio 89',
			7	: 'pm8901 sec / gpio 90',
			8	: 'pm8901 usr / gpio 91',
			9	: 'gsbi 1_2 uart rx data / gpio 34',
			10	: 'gsbi 2_2 uart rx data / gpio 38',
			11	: 'gsbi 3_2 uart rx data / gpio 42',
			12	: 'gsbi 4_2 uart rx data / gpio 46',
			13	: 'gsbi 5_2 uart rx data / gpio 50',
			14	: 'gsbi 6_2 uart rx data / gpio 54',
			15	: 'gsbi 7_2 uart rx data / gpio 58',
			16	: 'gsbi 8_2 uart rx data / gpio 63',
			17	: 'sdc1 dat 1 / gpio 160',
			18	: 'sdc1 dat 3 / gpio 162',
			19	: 'sdc2 dat 1 / gpio 144',
			20	: 'sdc2 dat 3 / gpio 146',
			21	: 'sdc3 dat 1',
			22	: 'sdc3 dat 3',
			23	: 'sdc4 dat 1',
			24	: 'sdc4 dat 3',
			25	: 'usb phy 1',
			26	: 'svideo load present',
			27	: 'hdmi rcv det',
			28	: 'soft reset#',
			29	: 'gp crci trigger / gpio 123',
			30	: 'hdmi hot plug / gpio 172',
			31	: 'sdc5 dat 1 / gpio 99',
			32	: 'sdc5 dat 3 / gpio 96',
			33	: 'gsbi 9_2 uart rx data / gpio 67',
			34	: 'gsbi 10_2 uart rx data / gpio 71',
			35	: 'gsbi 11_2 uart rx data / gpio 105',
			36	: 'gsbi 12_2 uart rx data / gpio 117',
			37	: 'gpio 29',
			38	: 'gpio 30',
			39	: 'gpio 31',
			40	: 'gpio 37',
			41	: 'gpio 40',
			42	: 'gpio 41',
			43	: 'gpio 45',
			44	: 'gpio 51',
			45	: 'gpio 52',
			46	: 'gpio 57',
			47	: 'gpio 73',
			48	: 'gpio 93',
			49	: 'gpio 94',
			50	: 'gpio 103',
			51	: 'gpio 104',
			52	: 'gpio 106',
			53	: 'gpio 115',
			54	: 'gpio 124',
			55	: 'gpio 125',
			56	: 'gpio 126',
			57	: 'gpio 127',
			58	: 'gpio 128',
			59	: 'gpio 129',
			60	: 'scorpion core 0 bringup',
			61	: 'scorpion core 1 bringup',
			62	: 'modem bringup',
			63	: 'lpass bringup',
		},
	},
	'8960' : {
		'resources' : {
			0	: '"Notifications"',
			1	: '"Request Invalidate"',
			2	: '"Timed Trigger"',
			3	: '"RPM CTL"',

			5	: '"CXO"',
			6	: '"PXO"',

			8	: '"Apps Fabric Clock"',
			9	: '"System Fabric Clock"',
			10	: '"MM Fabric Clock"',
			11	: '"Daytona Fabric Clock"',
			12  : '"SFPB Clock"',
			13  : '"CFPB Clock"',
			14  : '"MMFPB Clock"',

			16  : '"EBI1 Clock"',

			18  : '"Apps Fabric Halt"',
			19  : '"Apps Fabric Clock Mode"',
			20  : '"Apps Fabric IOCTL"',
			21  : '"Apps Fabric Arbitration"',
			22  : '"System Fabric Halt"',
			23  : '"System Fabric Clock Mode"',
			24  : '"System Fabric IOCTL"',
			25  : '"System Fabric Arbitration"',
			26  : '"MM Fabric Halt"',
			27  : '"MM Fabric Clock Mode"',
			28  : '"MM Fabric IOCTL"',
			29  : '"MM Fabric Arbitration"',

			30  : '"PM8921_S1"',
			31  : '"PM8921_S2"',
			32  : '"PM8921_S3"',
			33  : '"PM8921_S4"',
			34  : '"PM8921_S5"',
			35  : '"PM8921_S6"',
			36  : '"PM8921_S7"',
			37  : '"PM8921_S8"',
			38  : '"PM8921_L1"',
			39  : '"PM8921_L2"',
			40  : '"PM8921_L3"',
			41  : '"PM8921_L4"',
			42  : '"PM8921_L5"',
			43  : '"PM8921_L6"',
			44  : '"PM8921_L7"',
			45  : '"PM8921_L8"',
			46  : '"PM8921_L9"',
			47  : '"PM8921_L10"',
			48  : '"PM8921_L11"',
			49  : '"PM8921_L12"',
			50  : '"PM8921_L13"',
			51  : '"PM8921_L14"',
			52  : '"PM8921_L15"',
			53  : '"PM8921_L16"',
			54  : '"PM8921_L17"',
			55  : '"PM8921_L18"',
			56  : '"PM8921_L19"',
			57  : '"PM8921_L20"',
			58  : '"PM8921_L21"',
			59  : '"PM8921_L22"',
			60  : '"PM8921_L23"',
			61  : '"PM8921_L24"',
			62  : '"PM8921_L25"',
			63  : '"PM8921_L26"',
			64  : '"PM8921_L27"',
			65  : '"PM8921_L28"',
			66  : '"PM8921_L29"',
			67  : '"PM8921_CLK1"',
			68  : '"PM8921_CLK2"',
			69  : '"PM8921_LVS1"',
			70  : '"PM8921_LVS2"',
			71  : '"PM8921_LVS3"',
			72  : '"PM8921_LVS4"',
			73  : '"PM8921_LVS5"',
			74  : '"PM8921_LVS6"',
			75  : '"PM8921_LVS7"',

			80  : '"PM8921_NCP"',
			81  : '"CXO_BUFFERS"',
			82  : '"USB_OTG_SWITCH"',
			83  : '"HDMI_SWITCH"',
		},
		'masters' : {
			0	: '"APSS"',
			1	: '"MSS SW"',
			2	: '"LPASS"',
			3	: '"RIVA"',
			4	: '"DSPS"',
		},
		'mpm_ints' : {
			0	: 'timetick',
			1	: 'touchscreen pen irq (gpio 46)',
			2	: 'spare (reserved)',
			3	: 'rpm grouped interrupt',
			4	: 'PM8921 secure irq (gpio 103)',
			5	: 'PM8921 user irq (gpio 104)',
			6	: 'PM8921 modem irq (gpio 105)',
			7	: 'PM8018 secure irq (gpio 106)',
			8	: 'PM8018 user irq (gpio 107)',
			9	: 'gsbi 1_2 uart rx data (gpio 7)',
			10	: 'gsbi 2_2 uart rx data (gpio 11)',
			11  : 'gsbi 3_2 uart rx data (gpio 15)',
			12  : 'gsbi 4_2 uart rx data (gpio 19)',
			13  : 'gsbi 5_2 uart rx data (gpio 23)',
			14  : 'gsbi 6_2 uart rx data (gpio 27)',
			15  : 'gsbi 7_2 uart rx data (gpio 31)',
			16  : 'gsbi 8_2 uart rx data (gpio 35)',
			17  : 'sdc1 dat1',
			18  : 'sdc1 dat3',
			19  : 'sdc2 dat1 (gpio 90)',
			20  : 'sdc2 dat3 (gpio 92)',
			21  : 'sdc3 dat1',
			22  : 'sdc3 dat3',
			23  : 'sdc4 dat1 (gpio 85)',
			24  : 'sdc4 dat3 (gpio 83)',
			25  : 'usb phy irq0 (usb2_phy_id)',
			26  : 'svideo load present wakeup',
			27  : 'hdmi rcv det',
			28  : 'srst_n',
			29  : 'gp_crci_trigger (gpio 10)',
			30  : 'hdmi hot plut detect',
			31  : 'sdc5 dat1 (gpio 81)',
			32  : 'sdc5 dat3 (gpio 78)',
			33  : 'gsbi 9_2 uart rx data (gpio 94)',
			34  : 'gsbi 10_2 uart rx data (gpio 72)',
			35  : 'gsbi 11_2 uart rx data (gpio 39)',
			36  : 'gsbi 12_2 uart rx data (gpio 43)',
			37  : 'slimbus0_data (gpio 61)',
			38  : 'slimbus1_data (gpio 50)',
			39  : 'slimbus2_data (gpio 42)',
			40  : 'usb phy irq1 (usb2_phy_otgsessvld)',
			41  : 'tabla (gpio 62)',
			42  : 'gpio 76',
			43  : 'gpio 75',
			44  : 'gpio 70',
			45  : 'gpio 69',
			46  : 'gpio 67',
			47  : 'gpio 65',
			48  : 'gpio 58',
			49  : 'gpio 54',
			50  : 'gpio 52',
			51  : 'gpio 49',
			52  : 'gpio 40',
			53  : 'gpio 37',
			54  : 'gpio 24',
			55  : 'gpio 14',
			56  : 'spare (reserved)',
			57  : 'DSPS bringup request',
			58  : 'RIVA bringup request',
			59  : 'Q6FW bringup request',
			60  : 'KPSS0 bringup request',
			61  : 'KPSS1 bringup request',
			62  : 'Q6SW bringup request',
			63  : 'LPASS bringup request',
		},
	},
    '8930' : {
		'resources' : {
			0	: '"Notifications"',
			1	: '"Request Invalidate"',
			2	: '"Timed Trigger"',
			3	: '"RPM CTL"',

			5	: '"CXO"',
			6	: '"PXO"',
			7	: '"QDSS"',

			8	: '"Apps Fabric Clock"',
			9	: '"System Fabric Clock"',
			10	: '"MM Fabric Clock"',
			11	: '"Daytona Fabric Clock"',
			12  : '"SFPB Clock"',
			13  : '"CFPB Clock"',
			14  : '"MMFPB Clock"',

			16  : '"EBI1 Clock"',

			18  : '"Apps Fabric Halt"',
			19  : '"Apps Fabric Clock Mode"',
			20  : '"Apps Fabric IOCTL"',
			21  : '"Apps Fabric Arbitration"',
			22  : '"System Fabric Halt"',
			23  : '"System Fabric Clock Mode"',
			24  : '"System Fabric IOCTL"',
			25  : '"System Fabric Arbitration"',
			26  : '"MM Fabric Halt"',
			27  : '"MM Fabric Clock Mode"',
			28  : '"MM Fabric IOCTL"',
			29  : '"MM Fabric Arbitration"',

			30  : '"PM8038_S1"',
			31  : '"PM8038_S2"',
			32  : '"PM8038_S3"',
			33  : '"PM8038_S4"',
			34  : '"PM8038_S5"',
			35  : '"PM8038_S6"',
			36  : '"PM8038_L1"',
			37  : '"PM8038_L2"',
			38  : '"PM8038_L3"',
			39  : '"PM8038_L4"',
			40  : '"PM8038_L5"',
			41  : '"PM8038_L6"',
			42  : '"PM8038_L7"',
			43  : '"PM8038_L8"',
			44  : '"PM8038_L9"',
			45  : '"PM8038_L10"',
			46  : '"PM8038_L11"',
			47  : '"PM8038_L12"',
			48  : '"PM8038_L13"',
			49  : '"PM8038_L14"',
			50  : '"PM8038_L15"',
			51  : '"PM8038_L16"',
			52  : '"PM8038_L17"',
			53  : '"PM8038_L18"',
			54  : '"PM8038_L19"',
			55  : '"PM8038_L20"',
			56  : '"PM8038_L21"',
			57  : '"PM8038_L22"',
			58  : '"PM8038_L23"',
			59  : '"PM8038_L24"',
			60  : '"PM8038_L25"',
			61  : '"PM8038_L26"',
			62  : '"PM8038_L27"',
			63  : '"PM8038_LVS1"',
			64  : '"PM8038_LVS2"',
			65  : '"PM8038_CLK1"',
			66  : '"PM8038_CLK2"',

			80  : '"PM8921_NCP"',
			81  : '"CXO_BUFFERS"',
			82  : '"USB_OTG_SWITCH"',
			83  : '"HDMI_SWITCH"',
			84	: '"DDR_DMM"',
			85	: '"EBI1_CH0_RANGE"',
			86	: '"EBI1_CH1_RANGE"',
			87	: '"VDDCX_CORNER"',
		},
		'masters' : {
			0	: '"APSS"',
			1	: '"MSS SW"',
			2	: '"LPASS"',
			3	: '"RIVA"',
			4	: '"DSPS"',
		},
		'mpm_ints' : {
			0	: 'timetick',
			1	: 'touchscreen pen irq (gpio 46)',
			2	: 'spare (reserved)',
			3	: 'rpm grouped interrupt',
			4	: 'PM8921 secure irq (gpio 103)',
			5	: 'PM8921 user irq (gpio 104)',
			6	: 'PM8921 modem irq (gpio 105)',
			7	: 'PM8018 secure irq (gpio 106)',
			8	: 'PM8018 user irq (gpio 107)',
			9	: 'gsbi 1_2 uart rx data (gpio 7)',
			10	: 'gsbi 2_2 uart rx data (gpio 11)',
			11  : 'gsbi 3_2 uart rx data (gpio 15)',
			12  : 'gsbi 4_2 uart rx data (gpio 19)',
			13  : 'gsbi 5_2 uart rx data (gpio 23)',
			14  : 'gsbi 6_2 uart rx data (gpio 27)',
			15  : 'gsbi 7_2 uart rx data (gpio 31)',
			16  : 'gsbi 8_2 uart rx data (gpio 35)',
			17  : 'sdc1 dat1',
			18  : 'sdc1 dat3',
			19  : 'sdc2 dat1 (gpio 90)',
			20  : 'sdc2 dat3 (gpio 92)',
			21  : 'sdc3 dat1',
			22  : 'sdc3 dat3',
			23  : 'sdc4 dat1 (gpio 85)',
			24  : 'sdc4 dat3 (gpio 83)',
			25  : 'usb phy irq0 (usb2_phy_id)',
			26  : 'svideo load present wakeup',
			27  : 'hdmi rcv det',
			28  : 'srst_n',
			29  : 'gp_crci_trigger (gpio 10)',
			30  : 'hdmi hot plut detect',
			31  : 'sdc5 dat1 (gpio 81)',
			32  : 'sdc5 dat3 (gpio 78)',
			33  : 'gsbi 9_2 uart rx data (gpio 94)',
			34  : 'gsbi 10_2 uart rx data (gpio 72)',
			35  : 'gsbi 11_2 uart rx data (gpio 39)',
			36  : 'gsbi 12_2 uart rx data (gpio 43)',
			37  : 'slimbus0_data (gpio 61)',
			38  : 'slimbus1_data (gpio 50)',
			39  : 'slimbus2_data (gpio 42)',
			40  : 'usb phy irq1 (usb2_phy_otgsessvld)',
			41  : 'tabla (gpio 62)',
			42  : 'gpio 76',
			43  : 'gpio 75',
			44  : 'gpio 70',
			45  : 'gpio 69',
			46  : 'gpio 67',
			47  : 'gpio 65',
			48  : 'gpio 58',
			49  : 'gpio 54',
			50  : 'gpio 52',
			51  : 'gpio 49',
			52  : 'gpio 40',
			53  : 'gpio 37',
			54  : 'gpio 24',
			55  : 'gpio 14',
			56  : 'spare (reserved)',
			57  : 'DSPS bringup request',
			58  : 'RIVA bringup request',
			59  : 'Q6FW bringup request',
			60  : 'KPSS0 bringup request',
			61  : 'KPSS1 bringup request',
			62  : 'Q6SW bringup request',
			63  : 'LPASS bringup request',
		},
	},
    '8064' : {
		'resources' : {
			0	: '"Notifications"',
			1	: '"Request Invalidate"',
			2	: '"Timed Trigger"',
			3	: '"RPM CTL"',

			5	: '"CXO"',
			6	: '"PXO"',

			8	: '"Apps Fabric Clock"',
			9	: '"System Fabric Clock"',
#			10	: '"MM Fabric Clock"',
			10	: '"NSS0 Fabric Clock"',
			11	: '"Daytona Fabric Clock"',
			12  : '"SFPB Clock"',
			13  : '"CFPB Clock"',
#			14  : '"MMFPB Clock"',
			14  : '"NSS1 Clock"',

			16  : '"EBI1 Clock"',

			18  : '"Apps Fabric Halt"',
			19  : '"Apps Fabric Clock Mode"',
			20  : '"Apps Fabric IOCTL"',
			21  : '"Apps Fabric Arbitration"',
			22  : '"System Fabric Halt"',
			23  : '"System Fabric Clock Mode"',
			24  : '"System Fabric IOCTL"',
			25  : '"System Fabric Arbitration"',
			26  : '"MM Fabric Halt"',
			27  : '"MM Fabric Clock Mode"',
			28  : '"MM Fabric IOCTL"',
			29  : '"MM Fabric Arbitration"',

			30  : '"PM8921_S1"',
			31  : '"PM8921_S2"',
			32  : '"PM8921_S3"',
			33  : '"PM8921_S4"',
			34  : '"PM8921_S5"',
			35  : '"PM8921_S6"',
			36  : '"PM8921_S7"',
			37  : '"PM8921_S8"',
			38  : '"PM8921_L1"',
			39  : '"PM8921_L2"',
			40  : '"PM8921_L3"',
			41  : '"PM8921_L4"',
			42  : '"PM8921_L5"',
			43  : '"PM8921_L6"',
			44  : '"PM8921_L7"',
			45  : '"PM8921_L8"',
			46  : '"PM8921_L9"',
			47  : '"PM8921_L10"',
			48  : '"PM8921_L11"',
			49  : '"PM8921_L12"',
			50  : '"PM8921_L13"',
			51  : '"PM8921_L14"',
			52  : '"PM8921_L15"',
			53  : '"PM8921_L16"',
			54  : '"PM8921_L17"',
			55  : '"PM8921_L18"',
			56  : '"PM8921_L19"',
			57  : '"PM8921_L20"',
			58  : '"PM8921_L21"',
			59  : '"PM8921_L22"',
			60  : '"PM8921_L23"',
			61  : '"PM8921_L24"',
			62  : '"PM8921_L25"',
			63  : '"PM8921_L26"',
			64  : '"PM8921_L27"',
			65  : '"PM8921_L28"',
			66  : '"PM8921_L29"',
			67  : '"PM8921_CLK1"',
			68  : '"PM8921_CLK2"',
			69  : '"PM8921_LVS1"',
			70  : '"PM8921_LVS2"',
			71  : '"PM8921_LVS3"',
			72  : '"PM8921_LVS4"',
			73  : '"PM8921_LVS5"',
			74  : '"PM8921_LVS6"',
			75  : '"PM8921_LVS7"',
			76  : '"PM8821_S1"',
			77  : '"PM8821_S2"',
			78  : '"PM8821_L1"',

			80  : '"PM8921_NCP"',
			81  : '"CXO_BUFFERS"',
			82  : '"USB_OTG_SWITCH"',
			83  : '"HDMI_SWITCH"',






            90  : '"VDD_CX"',
            91  : '"VDD_UBI"',
            92  : '"VDD_APC0"',
            93  : '"VDD_APC1"',
            94  : '"NSSFPB clock"',
		},
		'masters' : {
			0	: '"APSS"',
			1	: '"GSS"',
			2	: '"LPASS"',
			3	: '"RIVA"',
			4	: '"DSPS"',
		},
		'mpm_ints' : {
			0	: 'timetick',
			1	: 'touchscreen pen irq (gpio 26)',
			2	: 'hsic wakeup (gpio 88)',
			3	: 'rpm grouped interrupt',
			4	: 'PM8921 secure irq (gpio 73)',
			5	: 'PM8921 user irq (gpio 74)',
			6	: 'PM8921 modem irq (gpio 75)',
			7	: 'PM8821 secure irq (gpio 76)',
			8	: 'PM8821 user irq (gpio 77)',
			9	: 'gpio 36',
			10	: 'pcie_hot_plug (gpio 84)',
			11  : 'gsbi 3_2 uart rx data (gpio 7)',
			12  : 'gsbi 4_2 uart rx data (gpio 11)',
			13  : 'gsbi 5_2 uart rx data (gpio 52)',
			14  : 'gsbi 6_2 uart rx data (gpio 15)',
			15  : 'gsbi 7_2 uart rx data (gpio 83)',
			16  : 'usb phy irq0 (usb3_phy_id)',
			17  : 'sdc1 dat1',
			18  : 'sdc1 dat3',
			19  : 'sdc2 dat1 (gpio 61)',
			20  : 'sdc2 dat3 (gpio 58)',
			21  : 'sdc3 dat1',
			22  : 'sdc3 dat3',
			23  : 'sdc4 dat1 (gpio 65)',
			24  : 'sdc4 dat3 (gpio 63)',
			25  : 'usb phy irq0 (usb1_phy_id)',
			26  : 'SPARE - RESERVED',
			27  : 'hdmi rcv det',
			28  : 'srst_n',
			29  : 'gp_crci_trigger (gpio 22)',
			30  : 'hdmi hot plut detect',
			31  : 'usb phy irq0 (usb4_phy_id)',
			32  : 'usb phy irq1 (usb4_phy_otgsessvld)',
			33  : 'gpio 44',
			34  : 'gpio 39',
			35  : 'gsbi 1_2 uart rx data (gpio 19)',
			36  : 'gsbi 2_2 uart rx data (gpio 23)',
			37  : 'slimbus0_data (gpio 41)',
			38  : 'slimbus1_data (gpio 30)',
			39  : 'usb phy irq1 (usb3_phy_otgsessvld)',
			40  : 'usb phy irq1 (usb1_phy_otgsessvld)',
			41  : 'tabla (gpio 42)',
			42  : 'gpio 56',
			43  : 'gpio 55',
			44  : 'gpio 50',
			45  : 'gpio 49',
			46  : 'gpio 47',
			47  : 'gpio 45',
			48  : 'gpio 38',
			49  : 'gpio 34',
			50  : 'gpio 32',
			51  : 'gpio 29',
			52  : 'gpio 18',
			53  : 'gpio 10',
			54  : 'gpio 81',
			55  : 'gpio 6',
			56  : 'KPSS2 bringup request',
			57  : 'DSPS bringup request',
			58  : 'RIVA bringup request',
			59  : 'KPSS3 bringup request',
			60  : 'KPSS0 bringup request',
			61  : 'KPSS1 bringup request',
			62  : 'GSS bringup request',
			63  : 'LPASS bringup request',
		},
	},


}

client_data = {
    'clients' : {
        0 :  'CLKRGM_SMB_CLIENT_NULL',
        1 :  'CLKRGM_SMB_CLIENT_MCPU',
        2 :  'CLKRGM_SMB_CLIENT_GLOBAL',
        3 :  'CLKRGM_SMB_CLIENT_FABRIC',
        4 :  'CLKRGM_SMB_CLIENT_ADSP',
        5 :  'CLKRGM_SMB_CLIENT_ACPU',
        6 :  'CLKRGM_SMB_CLIENT_GRP',
        7 :  'CLKRGM_SMB_CLIENT_GRP_2D',
        8 :  'CLKRGM_SMB_CLIENT_GRP_2D_1',
        9 :  'CLKRGM_SMB_CLIENT_MDP',
       10 :  'CLKRGM_SMB_CLIENT_MDP_VSYNC',
       11 :  'CLKRGM_SMB_CLIENT_CAMERA',
       12 :  'CLKRGM_SMB_CLIENT_CSI',
       13 :  'CLKRGM_SMB_CLIENT_DSI',
       14 :  'CLKRGM_SMB_CLIENT_DSI_PIXEL',
       15 :  'CLKRGM_SMB_CLIENT_IJPEG',
       16 :  'CLKRGM_SMB_CLIENT_JPEGD',
       17 :  'CLKRGM_SMB_CLIENT_PIXEL',
       18 :  'CLKRGM_SMB_CLIENT_ROTATOR',
       19 :  'CLKRGM_SMB_CLIENT_TV',
       20 :  'CLKRGM_SMB_CLIENT_VCODEC',
       21 :  'CLKRGM_SMB_CLIENT_VPE',
       22 :  'CLKRGM_SMB_CLIENT_VFE',
       23 :  'CLKRGM_SMB_CLIENT_USB_HS1',
       24 :  'CLKRGM_SMB_CLIENT_MODEM_OFFLINE',
       25 :  'CLKRGM_SMB_CLIENT_EBI0',
       26 :  'CLKRGM_SMB_CLIENT_EBI1',
       27 :  'CLKRGM_SMB_CLIENT_SMI',
       28 :  'CLKRGM_SMB_CLIENT_CFPB',
       29 :  'CLKRGM_SMB_CLIENT_SFPB',
       30 :  'CLKRGM_SMB_CLIENT_DFAB',
       31 :  'CLKRGM_SMB_CLIENT_CCPU',
       32 :  'CLKRGM_SMB_CLIENT_NSS0FAB',
       33 :  'CLKRGM_SMB_CLIENT_NSS1FAB',
       34 :  'CLKRGM_SMB_CLIENT_SFAB',
       35 :  'CLKRGM_SMB_CLIENT_AFAB',
       36 :  'CLKRGM_SMB_CLIENT_QDSS',
       37 :  'CLKRGM_SMB_CLIENT_SMB_UBI',
       38 :  'CLKRGM_SMB_CLIENT_NSSFPB',
       39 :  'CLKRGM_NUM_OF_SMB_CLIENTS',

     }
}

level_volt_data = {
    'level' : {
        0 :  80000,
        1 : 110000,
        2 : 115000,
        3 : 125000,
    }
}

pmic_rails = {
    'rails' : {
        0 : 'IPQ_PMIC_VDD_APC0',
        1 : 'IPQ_PMIC_VDD_APC1',
        2 : 'IPQ_PMIC_VDD_UBI',
        3 : 'IPQ_PMIC_VDD_CX',
    }
}

pmic_rail_clients = {
    'pclients' : {
        0 : 'IPQ_PMIC_CLIENTS_APC0',
        1 : 'IPQ_PMIC_CLIENTS_APC1',
        2 : 'IPQ_PMIC_CLIENTS_UBI',
        3 : 'IPQ_PMIC_CLIENTS_VCX',
        4 : 'IPQ_PMIC_CLIENTS_FABRIC',
    }
}
def get_level_voltage_value(volt):
    try:
        return level_volt_data['level'][volt]
    except KeyError:
        return '"Unknown level %i"' % level

def get_pmic_client_name(pclient):
    try:
        return pmic_rail_clients['pclients'][pclient]
    except KeyError:
        return '"Unknown client %i"' % pclient

def get_pmic_rail_name(rail):
    try:
        return pmic_rails['rails'][rail]
    except KeyError:
        return '"Unknown client %i"' % rail
def get_smb_client_name(client):
    try:
        return client_data['clients'][client]
    except KeyError:
        return '"Unknown client %i"' % client

def get_resource_name(resource):
	try:
		return target_data['resources'][resource]
	except KeyError:
		return '"Unknown resource %i"' % resource

def get_master_name(master):
	try:
		return target_data['masters'][master]
	except KeyError:
		return '"Unknown master %i"' % master

def get_interrupt_name(interrupt):
	rpm_interrupt_ids = {
		0	: '"SPM Shutdown Handshake"',
		1	: '"SPM Bringup Handshake"',
	}
	try:
		return rpm_interrupt_ids[interrupt]
	except KeyError:
		return '"Unknown interrupt %i"' % interrupt

def get_set_name(set):
	rpm_set_ids = {
		0	: '"Active Set"',
		1	: '"Sleep Set"',
	}
	try:
		return rpm_set_ids[set]
	except KeyError:
		return '"Unknown set %i"' % set

def decode_bitfield(name, bit_definitions, data):
	known_bits = 0
	for id in bit_definitions:
		known_bits |= (1 << id)
	unknown_data = data - (data & known_bits)
	string = ' | '.join(['[' + bit_definitions[x] + ']' for x in bit_definitions if (1 << x) & data])
	if unknown_data:
		if string:
			string += ' ...and '
		multi_or_single = ''
		if log(unknown_data, 2) != int(log(unknown_data, 2)):
			multi_or_single = 's'
		string += 'unknown %s%s 0x%0.8x' % (name, multi_or_single, unknown_data)
	return string

def get_action_names(actions):
	rpm_action_ids = {
		0	: 'Request',
		1	: 'Notification',
	}
	return decode_bitfield('action', rpm_action_ids, actions)

def get_interrupt_names(interrupts):
	return decode_bitfield('interrupt', target_data['mpm_ints'], interrupts)


#
# Log message parsers
#

class RPMBootStarted:
	__metaclass__ = Parser
	id = 0x00
	def parse(self, data):
		return 'rpm_boot_started'

class RPMBootFinished:
	__metaclass__ = Parser
	id = 0x01
	def parse(self, data):
		return 'rpm_boot_finished (succeeded: %i)' % (data[0] == 0)

class RPMDriverRegistered:
	__metaclass__ = Parser
	id = 0x02
	def parse(self, data):
		return 'rpm_driver_registered (resource: %s) (npa_driver: %i)' % (get_resource_name(data[0]), data[1])

class RPMDriverDeregistered:
	__metaclass__ = Parser
	id = 0x03
	def parse(self, data):
		return 'rpm_driver_deregistered (resource: %s)' % (get_resource_name(data[0]))

class RPMStatusUpdating:
	__metaclass__ = Parser
	id = 0x04
	def parse(self, data):
		return 'rpm_status_updating (resource: %s) (in_flux: %i)' % (get_resource_name(data[0]), data[1])

class RPMMessageIntReceived:
	__metaclass__ = Parser
	id = 0x05
	def parse(self, data):
		return 'rpm_message_int_received (master: %s)' % (get_master_name(data[0]))

class RPMServicingMaster:
	__metaclass__ = Parser
	id = 0x06
	def parse(self, data):
		states = ['idle', 'pre_dispatch', 'dispatch', 'post_dispatch']
		return 'rpm_servicing_master (master: %s) (state: %s) (preempt: %d) (stop_time: 0x%x)' % (get_master_name(data[0]), states[data[1]], data[2], data[3])

class RPMRequestAborted:
	__metaclass__ = Parser
	id = 0x07
	def parse(self, data):
		abort_reason = {
			0	: '"version mismatch"',
			1	: '"invalid parameters"',
			2	: '"master not ready"',
		}
		return 'rpm_request_aborted (master: %s) (reason: %s)' % (get_master_name(data[0]), abort_reason[data[1]])

class RPMReservedFieldWarning:
	__metaclass__ = Parser
	id = 0x08
	def parse(self, data):
		return 'rpm_reserved_field_warning'

class RPMDriverDispatch:
	__metaclass__ = Parser
	id = 0x09
	def parse(self, data):
		formatted = 'rpm_driver_dispatch (resource: %s)' % (get_resource_name(data[0]))
		if len(data) > 1:
			if data[1] == 1:
				formatted += ' (effective_immediately: yes)'
			elif data[1] == 2:
				formatted += ' (effective_immediately: no)'
		return formatted


class RPMDriverComplete:
	__metaclass__ = Parser
	id = 0x0a
	def parse(self, data):
		return 'rpm_driver_complete (rejected: %i)' % (data[0])

class RPMNoDriver:
	__metaclass__ = Parser
	id = 0x0b
	def parse(self, data):
		return 'rpm_no_driver'

class RPMSendingMessageInt:
	__metaclass__ = Parser
	id = 0x0c
	def parse(self, data):
		return 'rpm_sending_message_int (master: %s)\n' % (get_master_name(data[0]))

class RPMMasterDeferred:
	__metaclass__ = Parser
	id = 0x0d
	def parse(self, data):
		return 'rpm_master_deferred (master: %s) (actions: "%s") (defer_count: %i)' % (get_master_name(data[0]), get_action_names(data[1]), data[2])

class RPMMasterDeferralComplete:
	__metaclass__ = Parser
	id = 0x0e
	def parse(self, data):
		return 'rpm_master_deferral_complete (master: %s)' % (get_master_name(data[0]))

class RPMTriggerFired:
	__metaclass__ = Parser
	id = 0x0f
	def parse(self, data):
		return 'rpm_trigger_fired (interrupt: %s)' % (get_interrupt_name(data[0]))

class RPMTimedTriggerFired:
	__metaclass__ = Parser
	id = 0x10
	def parse(self, data):
		return 'rpm_timed_trigger_fired (master: %s)' % (get_master_name(data[0]))

class RPMBringupReq:
	__metaclass__ = Parser
	id = 0x11
	def parse(self, data):
		message = 'rpm_bringup_req (master: %s)' % (get_master_name(data[0]))
		if len(data) > 1:
			message += ' (core: %i)' % data[1]
		return message

class RPMShutdownReq:
	__metaclass__ = Parser
	id = 0x12
	def parse(self, data):
		message = 'rpm_shutdown_req (master: %s)' % (get_master_name(data[0]))
		if len(data) > 1:
			message += ' (core: %i)' % data[1]
		return message

class RPMBringupAck:
	__metaclass__ = Parser
	id = 0x13
	def parse(self, data):
		message = 'rpm_bringup_ack (master: %s)' % (get_master_name(data[0]))
		if len(data) > 1:
			message += ' (core: %i)' % data[1]
		return message

class RPMShutdownAck:
	__metaclass__ = Parser
	id = 0x14
	def parse(self, data):
		message = 'rpm_shutdown_ack (master: %s)' % (get_master_name(data[0]))
		if len(data) > 1:
			message += ' (core: %i)' % data[1]
		return message

class RPMMasterSetTransition:
	__metaclass__ = Parser
	id = 0x15
	def parse(self, data):
		return 'rpm_master_set_transition (master: %s) (leaving: %s) (entering: %s)' % (get_master_name(data[0]),
				get_set_name(data[1]), get_set_name(data[2]))

class RPMMasterSetTransition:
	__metaclass__ = Parser
	id = 0x16
	def parse(self, data):
		return 'rpm_set_transition_failure (resource: %s)' % (get_resource_name(data[0]))

class RPMErrorFatal:
	__metaclass__ = Parser
	id = 0x17
	def parse(self, data):
		return 'rpm_error_fatal (pc: 0x%0.8x) (a: 0x%0.8x) (b: 0x%0.8x) (c: 0x%0.8x)' % (data[0], data[1], data[2], data[3])

class RPMInvalidateToNothing:
	__metaclass__ = Parser
	id = 0x18
	def parse(self, data):
		return 'rpm_invalidate_to_nothing'

class RPMInvalidateToNothing:
	__metaclass__ = Parser
	id = 0x19
	def parse(self, data):
		return 'rpm_invalidated_driver_dispatch (resource: %s)' % (get_resource_name(data[0]))

class RPMInvalidateToNothing:
	__metaclass__ = Parser
	id = 0x1a
	def parse(self, data):
		return 'rpm_invalidated_driver_complete (rejected: %i)' % (data[0])

class RPMOutgoingNotification:
	__metaclass__ = Parser
	id = 0x1b
	def parse(self, data):
		return 'rpm_outgoing_notification (master: %s)' % (get_master_name(data[0]))

class RPMSoftRequestInitiated:
	__metaclass__ = Parser
	id = 0x1c
	def parse(self, data):
		return 'rpm_soft_request_initiated (master: %s) (resource: %s)' % (get_master_name(data[0]), get_resource_name(data[1]))

class RPMXOShutdown:
	__metaclass__ = Parser
	id = 0x1d
	def parse(self, data):
		if data[0]:
			return 'rpm_xo_shutdown_exit'
		else:
			return 'rpm_xo_shutdown_enter (count: %i) (planned_duration: 0x%0.8x)' % (data[1], data[2])

class RPMVddMin:
	__metaclass__ = Parser
	id = 0x1e
	def parse(self, data):
		if data[0] == 0:
			return 'rpm_vdd_min_enter (count: %i) (dig_mv: %i) (mem_mv: %i)' % (data[1], data[2], data[3])
		elif data[0] == 1:
			return 'rpm_vdd_min_exit'
		else:
			return 'rpm_vdd_min_enter (planned_duration: 0x%0.8x)' % (data[1])


class RPMMPMWakeupInts:
	__metaclass__ = Parser
	id = 0x1f
	def parse(self, data):
		return 'rpm_mpm_wakeup_ints (interrupts: "%s")' % (get_interrupt_names((data[1] << 32) | data[0]))

class RPMRequestQueued:
	__metaclass__ = Parser
	id = 0x20
	def parse(self, data):
		return 'rpm_request_queued (master: %s) (index: %i)' % (get_master_name(data[0]), data[1])

class RPMNoSleep:
	__metaclass__ = Parser
	id = 0x21
	def parse(self, data):
		message = 'rpm_no_sleep '
		if data[0] == 0:
			message += '(pending_int: %i)' % data[1]
		elif data[0] == 1:
			message += '(next_event: 0x%0.8x)' % (data[1])
		return message

class RPMTransitionQueued:
	__metaclass__ = Parser
	id = 0x22
	def parse(self, data):
		message = 'rpm_transition_queued (master: %s) ' % get_master_name(data[0])
		if data[1] == 0:
			message += '(scheduled: "no")'
		elif data[1]:
			message += '(scheduled: "yes") (deadline: 0x%0.8x)' % (data[2])
		return message

class RPMTransitionComplete:
	__metaclass__ = Parser
	id = 0x23
	def parse(self, data):
		return 'rpm_master_set_transition_complete (master: %s)' % get_master_name(data[0])

class RPMNewWorstCase:
	__metaclass__ = Parser
	id = 0x24
	def parse(self, data):
		if data[0] == 0:
			result = 'rpm_new_worst_case (resource: %s) (observed: %i)' % (get_resource_name(data[1]), data[2])
		else:
			result = 'rpm_new_worst_case (quantity: %i) (observed: %i)' % (data[1], data[2])
		if data[3] != 0:
			result += ' (frequency: %i)' % (data[3])
		return result

class RPMTimedTransition:
	__metaclass__ = Parser
	id = 0x25
	def parse(self, data):
		return 'rpm_timed_transition (master: %s) (from_set: %s) (to_set: %s) (deadline: 0x%0.8x)' % (get_master_name(data[0]), get_set_name(data[1]), get_set_name(data[2]), data[3])

class RPMHalt:
	__metaclass__ = Parser
	id = 0x26
	def parse(self, data):
		if data[0]:
			return 'rpm_halt_exit'
		else:
			return 'rpm_halt_enter (planned_duration: 0x%0.8x)' % (data[1])

class RPMTimetickRollover:
	__metaclass__ = Parser
	id = 0x27
	def parse(self, data):
		return 'rpm_timetick_rollover (current: 0x%0.8x) (last: 0x%0.8x) (upperBits: 0x%0.8x)' % (data[0], data[1], data[2])

class RPMBadTimetickRollover:
	__metaclass__ = Parser
	id = 0x28
	def parse(self, data):
		return 'rpm_bad_rollover (current: 0x%0.8x) (last: 0x%0.8x) (upperBits: 0x%0.8x)' % (data[0], data[1], data[2])

def parse_svs(mode, data):
	reasons = ['idle', 'speedup', 'no speedup', 'imminent processing', 'schedule is full', 'external vote']
	try:
		reason = reasons[data[0]]
	except KeyError:
		reason = 'unknown reason %i' % data[0]
	result = 'rpm_svs (mode: "%s") (reason: "%s")' % (mode, reason)
	if reason == 'speedup' or reason == 'no speedup':
		result += ' (old_duration: 0x%0.8x) (new_duration: 0x%0.8x) (switch_time: 0x%0.8x)' % (data[1], data[2], data[3])
	return result

class RPMSVSFast:
	__metaclass__ = Parser
	id = 0x29
	def parse(self, data):
		return parse_svs('fast', data)

class RPMSVSSlow:
	__metaclass__ = Parser
	id = 0x2a
	def parse(self, data):
		return parse_svs('slow', data)

class RPMHashMismatch:
	__metaclass__ = Parser
	id = 0x2b
	def parse(self, data):
		if data[0] == 0:
			result = ' (system_state: %d) (cache_result_state: %d) (result_state: %d)' % (data[1], data[2], data[3])
		else:
			result = ' (next_task: %d) (pre_state: %d) (next_state: %d) (system_hash: %d)' % (data[0], data[1], data[2], data[3])
		return 'rpm_hash_mismatch' + result

class RPMStopLogging:
	__metaclass__ = Parser
	id = 0x2c
	def parse(self, data):
		result = 'rpm_stop_logging (reason: %d)' % (data[0])
		return result

class RPMAbortedDispatch:
	__metaclass__ = Parser
	id = 0x2d
	def parse(self, data):
		if data[0] == 2:
			result = ' (master: %s) (estimate: 0x%x) (stop_time: 0x%x)' % (get_master_name(data[1]), data[2], data[3])
		elif data[0] == 1:
			result = ' (master: %s)' % (get_master_name(data[1]))
		return 'rpm_aborted_dispatch' + result

#
# NPA messages
#
class RPMNPARequestComplete:
	__metaclass__ = Parser
	id = 0x1000
	def parse(self, data):
		return '\t request complete (handle: 0x%08x) (sequence: 0x%08x) (request state:%d) (active state:%d)' \
					% (data[0], data[1], data[2], data[3])

class RPMNPAAssignResourceState:
	__metaclass__ = Parser
	id = 0x1001
	def parse(self, data):
		return 'npa_assign_resource_state (handle: 0x%08x) (resource: \"%s\") (active state: %d)' \
					% (data[0], npa_resource_lookup(data[0]), data[1])

class RPMNPAIssueLimitMaxRequest:
	__metaclass__ = Parser
	id = 0x1002
	def parse(self, data):
		return 'npa_issue_limit_max_request (handle: 0x%08x) (client: \"%s\") (max: %d) (resource: \"%s\")' \
					% (data[0], npa_client_name_lookup(data[0]), data[2], npa_resource_lookup(data[3]))

class RPMNPAIssueRequiredRequest:
	__metaclass__ = Parser
	id = 0x1003
	def parse(self, data):
		return 'npa_issue_required_request (handle: 0x%08x) (client: \"%s\") (request: %u) (resource: \"%s\")' \
					% (data[0], npa_client_name_lookup(data[0]), data[1], npa_client_resource_lookup(data[0]))

class RPMNPAModifyRequest:
	__metaclass__ = Parser
	id = 0x1004
	def parse(self, data):
		return 'npa_modify_request (handle: 0x%08x) (client: \"%s\") (delta: %d) (resource: \"%s\")' \
					% (data[0], npa_client_name_lookup(data[0]), data[1], npa_client_resource_lookup(data[0]))

class RPMNPAIssueImpulseRequest:
	__metaclass__ = Parser
	id = 0x1005
	def parse(self, data):
		return 'npa_issue_impulse_request (handle: 0x%08x) (client: \"%s\") (resource: \"%s\")' \
					% (data[0], npa_client_name_lookup(data[0]), npa_client_resource_lookup(data[0]))

class RPMNPAIssueVectorRequest:
	__metaclass__ = Parser
	id = 0x1006
	def parse(self, data):
		return 'npa_issue_vector_request (handle: 0x%08x) (client: \"%s\") (resource: \"%s\") (num_elems: %d) (vector: 0x%08x)' \
					% (data[0], npa_client_name_lookup(data[0]), npa_client_resource_lookup(data[0]), data[1], data[2])

class RPMNPAIssueInternalRequest:
	__metaclass__ = Parser
	id = 0x1007
	def parse(self, data):
		return 'npa_issue_internal_request (handle: 0x%08x) (client: \"%s\") (resource: \"%s\")' \
					% (data[0], npa_client_name_lookup(data[0]), npa_client_resource_lookup(data[0]))

class RPMNPACompleteRequest:
	__metaclass__ = Parser
	id = 0x1008
	def parse(self, data):
		return 'npa_complete_request (handle: 0x%08x) (client: \"%s\") (resource: \"%s\")' \
					% (data[0], npa_client_name_lookup(data[0]), npa_client_resource_lookup(data[0]))

class RPMNPACancelRequest:
	__metaclass__ = Parser
	id = 0x1009
	def parse(self, data):
		return 'npa_cancel_request (handle: 0x%08x) (client: \"%s\") (resource: \"%s\")' \
					% (data[0], npa_client_name_lookup(data[0]), npa_client_resource_lookup(data[0]))

class RPMNPAIssueRequiredRequest:
	__metaclass__ = Parser
	id = 0x100a
	def parse(self, data):
		return 'npa_queue_preemptive_flush'

class RPMNPAIssueRequiredRequest:
	__metaclass__ = Parser
	id = 0x100b
	def parse(self, data):
		return 'npa_issue_suppressible_request (handle: 0x%08x) (client: \"%s\") (request: %u) (resource: \"%s\")' \
					% (data[0], npa_client_name_lookup(data[0]), data[1], npa_client_resource_lookup(data[0]))


class RPMCPRPreSwitchEntry:
	__metaclass__ = Parser
	id = 0x2F
	def parse(self, data):
		return 'CPR Pre-switch Entry: (Input mode: %d ) (Input Level: %d uv)' \
					% (data[0], data[1])

class RPMCPRPreSwitchExit:
	__metaclass__ = Parser
	id = 0x30
	def parse(self, data):
		return 'CPR Pre-swtich Exit: (Output Mode: %d ) (Output Level : %d uv")' \
					% (data[0], data[1])


class RPMCPRPostSwitchEvent:
	__metaclass__ = Parser
	id = 0x31
	def parse(self, data):
		return 'CPR Post-switch Exit: (Current Mode: %d ) (Current Level : %d mv)' \
					% (data[0], data[1])

class RPMCPRNewMeasurement:
	__metaclass__ = Parser
	id = 0x32
	def parse(self, data):
		return 'CPR interrupt fired: (New offset: %d uv) (Current level : %d) (Current voltage: %d)' \
					% (data[0], data[1], data[2])

class RPMRailwayPreSwitch:
	__metaclass__ = Parser
	id = 0x33
	def parse(self, data):
		return 'Railway Pre-switch Recommendation: (Recommended mode: %d ) (Recommended level : %d")' \
					% (data[0], data[1])

class RPMRailwayPostSwitch:
	__metaclass__ = Parser
	id = 0x34
	def parse(self, data):
		return 'Railway Post-switch Recommendation: (Recommended mode: %d ) (Recommended level : %d")' \
					% (data[0], data[1])
					
class RPMRailwayDefault:
	__metaclass__ = Parser
	id = 0x35
	def parse(self, data):
		return 'Railway Default Recommendation: (Selected mode: %d ) (Selected level : %d")' \
					% (data[0], data[1])

class RPMRailwayNewProposal:
	__metaclass__ = Parser
	id = 0x36
	def parse(self, data):
		return 'Railway New Proposal: (Selected mode: %d ) (Selected level : %d")' \
					% (data[0], data[1])

class RPMResourceInfo:
	__metaclass__ = Parser
	id = 0x41
	def parse(self, data):
		return 'Resource Request Info: \" %s \"  (Initial Val: %d ) (Requested Val: %d")' \
					% (get_resource_name(data[0]), data[1], data[2])
class RPMIPQClkVoteInfo:
	__metaclass__ = Parser
	id = 0x42
	def parse(self, data):
		return 'SMB Vote Info: \" %s \"  (Initial level: %d ) (Requested Level: %d")' \
					% (get_smb_client_name(data[0]), get_level_voltage_value(data[1]), get_level_voltage_value(data[2]))
class RPMIPQPmicVoteInfo:
	__metaclass__ = Parser
	id = 0x43
	def parse(self, data):
		return 'IPQ_PMIC Vote Info: \" %s \"  (Initial level: %d ) (Requested Level: %d")' \
					% (get_pmic_client_name(data[0]), (data[1]), (data[2]))
class RPMIPQPmicClientVoteInfo:
	__metaclass__ = Parser
	id = 0x44
	def parse(self, data):
		return 'IPQ_PMIC Client Vote Info: \" %s \"  \" %s \" (Requested Level: %d")' \
					% (get_pmic_rail_name(data[0]),get_pmic_client_name(data[1]), (data[2]))
#                   
#
#
#
# The parsing core
#

if __name__ == '__main__':
	# Get command line arguments
	parser = OptionParser()
	parser.add_option('-f', '--file', dest='filename',
						help='RPM ULog file to parse', metavar='FILE')
	parser.add_option('-n', '--npa_dump', dest='npa_filename',
						help='NPA dump to resolve NPA names from', metavar='FILE')
	parser.add_option('-r', '--raw_timestamp', dest='raw_timestamp',
						help='Print timestamps as raw hex values', action='store_true', default=False)
	parser.add_option('-p', '--pretty_timestamp', dest='raw_timestamp',
						help='Print timestamps as pretty floating point values', action='store_false')
	parser.add_option('-x', '--high_precision', dest='hp_timestamp',
						help='Print time between messages calculated high precision timer', action='store_true', default=False)
	parser.add_option('-t', '--target', dest='target', default="8660",
						help='What target was this RPM build taken from (chip number only, e.g. "8660" or "8960")')
	(options, args) = parser.parse_args()
	if not options.filename:
		parser.error('-f option is required')

	if options.npa_filename:
		parse_npa_dump(options.npa_filename)

	if options.target not in target_data:
		print 'Error: unknown target %s' % options.target
		sys.exit(1)
	target_data = target_data[options.target]

	# Try to load data from the log file.
	try:
		f = open(options.filename)
		loglines = f.readlines()
		f.close()
	except:
		print 'Error loading log data from file:', sys.exc_info()[0]
		raise

	# Got data, parse it as well as we can.
	last_hp_timestamp = None
	for line in loglines:
		# First try to unpack it into its components.
		try:
			assert '- ' == line[0:2]
			bytestring = ''.join(map(lambda x: chr(int(x, 16)), line[2:line.rfind(',')].split(', ')))
			message = list(unpack('<%iL' % (len(bytestring)/4), bytestring))

			timestamp = message[0]
			id = message[1]
			data = message[2:]
		except:
			print 'Error parsing message from logfile:', sys.exc_info()[0]

		# Then try to find a parser for it.
		if options.raw_timestamp:
			timestamp = '0x%x' % timestamp
		else:
			try:
				timestamp = '%f' % (timestamp / 32768.0)
			except:
				import pdb; pdb.set_trace()

		if options.hp_timestamp:
			try:
				hp_timestamp = float(data[4])
				if last_hp_timestamp is not None:
					elapsed_ticks = hp_timestamp - last_hp_timestamp
					ticks_to_us = 0.148148148
					elapsed_us = elapsed_ticks * ticks_to_us
					timestamp += ' [+%8.3f us]' % elapsed_us
					timestamp += ' {%0.8x -> %0.8x}' % (last_hp_timestamp, hp_timestamp)
				last_hp_timestamp = hp_timestamp
			except:
				print 'This log does not contain high-precision timestamp data.'
				options.hp_timestamp = False
		try:
			pretty_message = parsers[id]().parse(data)
			print "%s: %s" % (timestamp, pretty_message)
		except:
			print 'Error parsing log message with timestamp = %s, id = %i, and data = %s -- %s' % (timestamp, id, data, sys.exc_info()[0])

