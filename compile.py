#!/usr/bin/python2
import sys, os, platform

lastCode = 0

if len(sys.argv) < 2:
	print "Usage: {0} command".format (sys.argv[0])
	print ""
	print "Commands:"
	print "	generate"
	print "	compile"
	print "	programm programmer [vendor product|device]"
	print "	build programmer [vendor product|device]"
	exit (1)

def parseBoolean (val):
	val = val.lower ()
	if val == "on": return True
	if val == "off": return False
	if val == "true": return True
	if val == "false": return False
	if val == "1": return True
	if val == "0": return False
	return None
def parseInteger (val):
	try:
		return int(val)
	except ValueError:
		return None

device_ = ""
arch_ = ""
optimize_ = "-O1"
files = []
defines = []
includes = []
options = []
linkers = []
speed_ = 115200

def source (path):
	global files
	files.append (path)
def define (d):
	global defines
	defines.append (d)
def include (inc):
	global includes
	includes.append (inc)
def option (inc):
	global options
	options.append (inc)
def linker (li):
	global linkers
	linkers.append (li)
def device (val):
	global device_, arch_
	device_ = val
	if "atmega" in device_:
		arch_ = "avr"
	if "stm32f4" in device_:
		arch_ = "arm-m4"
	if "stm32f1" in device_:
		arch_ = "arm-m3"
	if "stm32f0" in device_:
		arch_ = "arm-m0"
def arch (val):
	global arch_
	arch_ = val
def optimize (opt):
	global optimize_
	optimize_ = "-O" + opt
def speed (opt):
	global speed_
	speed_ = opt

if os.path.exists ("config.cfg"):
	execfile ("config.cfg")

def checkLink ():
	if not os.path.exists ("do"):
		file ("do", "wb").write ("#!/bin/bash\npython2 {0} $*\n".format (sys.argv[0]))
		os.chmod ("do", 0777)

##
def doGenerate ():
	file ("config.cfg", "wb").write ("""device ("stm32f100")
freq=8000000

source ("main.c")
#define ("")

include (".")

# AVR
#source ("/home/krystiand/prog/_uc/avr-public/usbdrv/usbdrvasm.S")

#include ("/home/krystiand/prog/_uc/avr-public")
#include ("/home/krystiand/prog/_uc/avr-public/usbdrv")
""")
	checkLink ()
	return 0

def doCompile ():
	checkLink ()
	
	if len(arch_) == 0:
		print "No arch specified. use arch()"
		exit (1)
	
	if not os.path.exists ("build"):
		os.mkdir ("build")
	
	if arch_[0:3] == "arm":
		PREFIX = "arm-none-eabi"
		# PREFIX = "arm-none-linux-gnueabi"
	elif arch_ == "avr":
		PREFIX = "avr"
	CC = PREFIX + "-gcc"
	OBJCOPY = PREFIX + "-objcopy"
	OBJDUMP = PREFIX + "-objdump"
	SIZE = PREFIX + "-size"
	
	CFLAGS = []
	LFLAGS = []
	OPTIONS = []
	
	if arch_ == "arm-m0":
		CFLAGS.append ("-mcpu=cortex-m0")
		CFLAGS.append ("-msoft-float")
		CFLAGS.append ("-mthumb")
		CFLAGS.append ("-nostartfiles")
	elif arch_ == "arm-m3":
		CFLAGS.append ("-mcpu=cortex-m3")
		CFLAGS.append ("-march=armv7-m")
		CFLAGS.append ("-mfix-cortex-m3-ldrd")
		CFLAGS.append ("-msoft-float")
		CFLAGS.append ("-mthumb")
		CFLAGS.append ("-nostartfiles")
	elif arch_ == "arm-m4":
		CFLAGS.append ("-mcpu=cortex-m4")
		# CFLAGS.append ("-march=armv7-m")
		# CFLAGS.append ("-mfix-cortex-m3-ldrd")
		CFLAGS.append ("-msoft-float")
		CFLAGS.append ("-mthumb")
		CFLAGS.append ("-nostartfiles")
	elif arch_ == "avr":
		CFLAGS.append ("-mmcu=" + device_)
	
	CFLAGS.append ("-ggdb")
	CFLAGS.append (optimize_)
	
	# if arch_ == "avr":
	# CFLAGS.append ("-fno-inline-small-functions")
	# CFLAGS.append ("-fno-split-wide-types")
	# CFLAGS.append ("-fno-tree-scev-cprop")
	# CFLAGS.append ("-fdata-sections")
	# CFLAGS.append ("-ffunction-sections")
	# LFLAGS.append ("--relax,--gc-sections")
	# LFLAGS.append ("-gc-sections")
	# pass
	
	defines.append ("F_CPU=" + str(freq))
	if arch_[0:3] == "arm":
		defines.append ("RAMSIZE=" + str(ramsize))
	
	for inc in includes: CFLAGS.append ("-I" + inc)
	for d in defines: CFLAGS.append ("-D" + d)
	for l in linkers: LFLAGS.append ("" + l)
	for o in options: OPTIONS.append (o);

	CFLAGS = " ".join ([c for c in CFLAGS])
	
	objs = []
	
	for path in files:
		objPath = "build/" + path.replace ("/", "_").replace ("..", "_").replace (".c", ".o").replace (".S", ".o")
		objs.append (objPath)
	
		cmd = "{CC} -c {CFLAGS} {file} -o {destPath}".format (
			CC=CC, CFLAGS=CFLAGS, OPTIONS=" ".join (OPTIONS), file=path, destPath=objPath)

		print cmd
		code = (os.system (cmd) >> 8) & 0xff
		if code != 0:
			return code

	lib = ""
	if arch_[0:3] == "arm":
		LFLAGS.append ("-Map=build/main.map")
		LFLAGS.append ("-T main.ld")
	
		# lib = "/usr/local/arm-none-eabi/lib/gcc/arm-none-eabi/4.7.1/libgcc.a"
		# if not os.path.exists (lib):
			# lib = "/home/krystiand/arm/lib/gcc/arm-none-eabi/4.7.1/libgcc.a"
	# lib = "/home/krystiand/gcc-arm-none-eabi-4_7-2012q4/arm-none-eabi/lib/armv7-m/libg.a"
	# lib = "/home/krystiand/gcc-arm-none-eabi-4_7-2012q4/lib/gcc/arm-none-eabi/4.7.3/armv7-m/libgcc.a"
	# lib += " /home/krystiand/gcc-arm-none-eabi-4_7-2012q4/arm-none-eabi/lib/armv7-m/libc.a"

	# lib += " /home/krystiand/gcc-arm-none-eabi-4_7-2012q4/arm-none-eabi/lib/armv7-m/libm.a"
	# lib += " /home/krystiand/Downloads/arm-2012.09/arm-none-eabi/lib/thumb2/libm.a"
	# lib += " /usr/local/codesourcery/arm-none-eabi/lib/thumb2/libm.a"
	# if arch_ == "avr":
	lib = "-lm -lc"

	LFLAGS = " ".join (["-Wl," + l for l in LFLAGS])

	destPath = "build/main.elf"
	cmd = "{CC} {LFLAGS} {CFLAGS} {files} {OPTIONS} {lib} -o {destPath}".format (
		CC=CC, LFLAGS=LFLAGS, CFLAGS=CFLAGS, OPTIONS=" ".join (OPTIONS), lib=lib, files=" ".join (objs), destPath=destPath)
	print cmd
	code = (os.system (cmd) >> 8) & 0xff
	if code != 0:
		return code

	cmd = "{OBJCOPY} -O binary build/main.elf build/main.bin".format (OBJCOPY=OBJCOPY)
	print cmd
	code = (os.system (cmd) >> 8) & 0xff
	if code != 0:
		return code
		
	cmd = "{OBJCOPY} -O ihex build/main.elf build/main.hex".format (OBJCOPY=OBJCOPY)
	print cmd
	code = (os.system (cmd) >> 8) & 0xff
	if code != 0:
		return code
		
	cmd = "{OBJDUMP} -h -S build/main.elf > build/main.lst".format (OBJDUMP=OBJDUMP)
	print cmd
	code = (os.system (cmd) >> 8) & 0xff
	if code != 0:
		return code
		
	cmd = "{SIZE} build/main.elf".format (SIZE=SIZE)
	print cmd
	code = (os.system (cmd) >> 8) & 0xff
	if code != 0:
		return code
	return 0

def doProgramm (programmator, vendor = "", product = "", device = "/dev/ttyUSB0"):
	if prog == "usbasp":
		cmd = "avrdude -p {device} -c usbasp -B 1 -U flash:w:build/main.hex".format (device=device_)
	elif prog == "avrisp" or prog == "avrispmkii":
		cmd = "avrdude -p {device} -c avrispmkii -P usb -U flash:w:build/main.hex".format (device=device_)
	elif prog == "flasher":
		cmd = "/home/krystiand/prog/_uc/uart_stm32_flasher/flasher {device} {speed} build/main.bin".format (device=device,speed=speed_)
	elif prog == "kd":
		if platform.machine () == "i386" or platform.machine () == "i686":
			cmd = "/home/krystiand/prog/_uc/usb_programm/usb_programm32 reset_programm"
		else:
			cmd = "/home/krystiand/prog/_uc/usb_programm/usb_programm --reset --write=build/main.hex"
		cmd += " --device={vendor}:{product}".format (vendor=vendor, product=product)
	print cmd
	return os.system (cmd)

command = sys.argv[1]
if command == "generate":
	lastCode = doGenerate ()

if command == "compile":		
	lastCode = doCompile ()

if command == "programm":
	if len(sys.argv) < 3:
		print "Usage: {0} programm programmator".format (sys.argv[0])
		exit (1)
	prog = sys.argv[2]
	if prog == "kd" and len(sys.argv) < 5:
		print "Usage: {0} programm kd vendor product".format (sys.argv[0])
		exit (1)

	vendor = product = device = ""
	if prog == "kd":
		vendor = sys.argv[3]
		product = sys.argv[4]
	if prog == "flasher":
		device = sys.argv[3]
	lastCode = doProgramm (prog, vendor, product, device)

if command == "build":
	if len(sys.argv) < 3:
		print "Usage: {0} build programmator".format (sys.argv[0])
		exit (1)
	prog = sys.argv[2]
	if prog == "kd" and len(sys.argv) < 5:
		print "Usage: {0} programm kd vendor product".format (sys.argv[0])
		exit (1)
	lastCode = doCompile ()
	if lastCode == 0:
		vendor = product = device = ""
		if prog == "kd":
			vendor = sys.argv[3]
			product = sys.argv[4]
		if prog == "flasher":
			device = sys.argv[3]
		lastCode = doProgramm (prog, vendor, product, device)
	else:
		print "error while compiling:", lastCode

exit (lastCode)
