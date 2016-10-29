#artTool python version , created at 2015-12-22 by GaoTianxing
import sys
import os
import time
import struct
import zlib
import ConfigParser
from ctypes import *
from AnalysisDump import *

def uninstallOri():
	return

def installApk():
	return	
	
def getOATandLib(apkpath, package, oatpath, transpath, targetfile):

	commandOne = 'adb shell su -c "cp %s%s %s"' %(oatpath, targetfile, transpath)
	commandTwo = 'adb pull %s%s' %(transpath, targetfile)
	commandThree = 'copy %s arthack' %targetfile

	print commandOne
	print commandTwo
	print commandThree

	ret = os.popen(commandOne).read()
	print ret
	ret = os.popen(commandTwo).read()
	print ret

	ret = os.popen(commandThree).read()
	print ret

	cmd = 'adb shell su -c "cp %s%s/lib/arm/libJniTest.so %s "' %(apkpath, package, transpath)
	print cmd
	ret = os.popen(cmd).read()
	print ret

	cmd = 'adb pull %slibJniTest.so' %transpath
	print cmd
	ret = os.popen(cmd).read()
	print ret
	
	#save the lib size
	lib = open('libJniTest.so', 'r+b')
	filelen = os.path.getsize('libJniTest.so')
	lib.close()
	
	return filelen

def parseOat(classname, password):
	dllfile = 'artTool.dll'

	artdeal = CDLL(dllfile)

	str = c_char_p(classname)
	pymethodsize = artdeal.getMethodSize
	pydealfile = artdeal.dealARTFile

	pydealfile.argtype = [c_char_p]
	pydealfile.restype = POINTER(c_int)

	method_size = pymethodsize(str) #method count in oat
	print method_size

	#method_ret = pointer(c_int(method_size * 2))
	method_ret = pydealfile(str)
	
	artdeal.cryptfile(password, 'codefile')

	return method_ret, method_size

def insertToLib(classname, password):

	lib = open('libJniTest.so', 'r+b')
	lib.seek(0, 2)

	code = open('codefile', 'r+b')
	filelen = os.path.getsize(codefile)
    
	namelen = len(classname)
	
	lib.write(struct.pack('i', filelen))
	lib.write(struct.pack('i', namelen))
	lib.write(classname)
	
	for i in range(0, filelen):
		b = code.read(1)
		lib.write(b)
	
	dllfile = 'artTool.dll'
	artdeal = CDLL(dllfile)
	artdeal.cryptfile(password, 'dexmethod')
	dexmethod = open('dexmethod', 'r+b')
	filelen = os.path.getsize('dexmethod')
	print 'dexmethod len' + str(filelen)
	lib.write(struct.pack('i', filelen))
	for i in range(0, filelen):
		b = dexmethod.read(1)
		lib.write(b)
	
	headerinfo = open('headerinfo', 'r+b')
	filelen = os.path.getsize('headerinfo')
	print 'headerinfo len' + str(filelen)
	lib.write(struct.pack('i', filelen))
	for i in range(0, filelen):
		b = headerinfo.read(1)
		lib.write(b)
	
	headerinfo.close()
	
	return

def modifyOat(targetfile, method_ret, method_size):

	while True:
		index = raw_input( 'Please enter the index code you want to clear 0 to exit: ');
		
		if int(index) == 0:
			break
		
		cut = 0;
		try:
			bitmap = open('bitmap');
			if bitmap != None:
				line = bitmap.readline()
				for i in range(0, int(index) - 1):
					if int(str(line[i])) == 0:
						cut = cut + 1
		except IOError, e:
			print 'no bitmap'
			
		target = int(index) - 1 - cut
		code_offset = method_ret[target * 2] + 1
		code_size = method_ret[target * 2 + 1] - 1

		modi = open(targetfile, 'r+b')
		modi.seek(code_offset)
		for i in range(0, code_size):
			modi.write(b'\x00')

		modi.close()
	
	dex_begin = method_ret[method_size * 2]
	return dex_begin

def getAfterApk(apkpath, pakage, transpath):
	cmd = 'adb shell su -c "cp %s%s/base.apk %s "' %(apkpath, pakage, transpath)
	print cmd
	ret = os.popen(cmd).read()
	print ret

	cmd = 'adb pull %sbase.apk' %transpath
	print cmd
	ret = os.popen(cmd).read()
	print ret

	return

def unpackApk():
	cmd = 'apktool d base.apk -p frameworkes -f -o arthack_unpack -s'
	print cmd
	ret = os.popen(cmd).read()
	print ret

	cmd = 'copy arthack_unpack\classes.dex ori_classes.dex'
	print cmd
	ret = os.popen(cmd).read()
	print ret

	cmd = 'copy arthack_unpack\classes.dex classes.dex'
	print cmd
	ret = os.popen(cmd).read()
	print ret

	return

def parseDex(classname):
	dllfile = 'artTool.dll'

	artdeal = CDLL(dllfile)
	str = c_char_p(classname)
	pymethodsize = artdeal.getMethodSizeDEX
	pydealdex = artdeal.dealDEXFile

	pydealdex.argtype = [c_char_p]
	pydealdex.restype = POINTER(c_int)

	method_size = pymethodsize(str)
	print method_size

	method_ret = pointer(c_int(method_size * 3))
	method_ret = pydealdex(str)
	
	return method_ret

def modifyDex(targetfile, method_ret, dex_begin):

	while True:
		index = raw_input( 'Please enter the index code you want to clear 0 to exit: ');
		
		if int(index) == 0:
			break
		
		target = int(index) - 1
		method_item = method_ret[target * 3]
		code_size = method_ret[target * 3 + 1] * 2
		code_offset = method_ret[target * 3 + 2]

		print code_offset
		
		modi = open('classes.dex', 'r+b')
		#findIndump(modi, code_offset, code_size, target)
		modi.seek(code_offset)
		modi.write(b'\x12\x21')
		modi.write(b'\x12\x20')
		for i in range(4, code_size / 2):
			modi.write(b'\x01\x10')

		modi.close()
		
		
		print code_offset + dex_begin
		
		#clear oat file dex code
		
		modi = open(targetfile, 'r+b')
		
		if os.path.exists('dexmethod') == False:
			f = open('dexmethod', 'w+b')
			f.close()
		
		dexmethod = open('dexmethod', 'r+b')
		dexmethod.seek(0, 2)
		dexmethod.write(struct.pack('i', code_size))
		dexmethod.write(struct.pack('i', int(index)))
		modi.seek(code_offset + dex_begin)
		for i in range(0, code_size):
			b = modi.read(1)
			dexmethod.write(b)

		modi.seek(0)
		
		#findIndump(modi, code_offset + dex_begin, code_size, target)
		findIndump(dexmethod, 8, code_size, target)
		
		dexmethod.close()
		modi.close()
		
	blocksize = 1024 * 64 
	f = open('classes.dex',"r+b")
	f.seek(12)
	ucrc = zlib.adler32(f.read()) 
	if ucrc>0:
		uoutint=ucrc
	else :
		uoutint= ~ ucrc ^ 0xffffffff	
	print uoutint
	byte = struct.pack('I', uoutint)
	f.seek(8)
	f.write(byte)
	f.close()

def repackApk(liblen):
		
	lib = open('libJniTest.so', 'r+b')
	lib.seek(0, 2)
	lib.write(struct.pack('i', liblen))
	
	image_record = open('imagerecord', 'r+b')
	for i in range(0, 4):
		b = image_record.read(1)
		lib.write(b)
	
	lib.close()

	cmd = 'copy /y libJniTest.so arthack_unpack\\lib\\armeabi\\libJniTest.so'
	ret = os.popen(cmd).read()
	print ret
	
	cmd = 'apktool b arthack_unpack -p frameworkes'
	ret = os.popen(cmd).read()
	print ret

	return

def correctCrc(targetfile):
	dllfile = 'artTool.dll'

	artdeal = CDLL(dllfile)

	zipcrc = artdeal.getZipDexCRC()
	crcoffset = artdeal.getOatCrcOffset()

	byte = struct.pack('i', zipcrc)

	modi = open(targetfile, 'r+b')
	modi.seek(crcoffset)
	modi.write(byte)

	return

def pushOat(transpath, targetfile, oatpath):
	commandSix = 'adb push %s %s' %(targetfile, transpath)
	print commandSix
	ret = os.popen(commandSix).readlines()

	commandSeven = 'adb shell su -c "cp %s%s %s "' %(transpath, targetfile, oatpath)
	print commandSeven
	ret = os.popen(commandSeven).readlines()

def pushApk(package, transpath):
	cmd = 'adb push arthack_unpack/dist/base.apk %s' %transpath
	ret = os.popen(cmd).read()
	print ret

	cmd = 'adb shell su -c "cp %sbase.apk %s%s/ "' %(transpath, apkpath, package)
	ret = os.popen(cmd).read()
	print ret

	return

def uploadCode(transpath, codefile, apkpath, package):
	commandFour = 'adb push %s %s' %(codefile, transpath)
	print commandFour
	ret = os.popen(commandFour).readlines()
	print ret

	commandFive = 'adb shell su -c "chmod 777 %s%s"' %(transpath, codefile)
	print commandFive
	ret = os.popen(commandFive).readlines()
	print ret

	commandFour = 'adb push %s %s' %('libJniTest.so', transpath)
	print commandFour
	ret = os.popen(commandFour).readlines()
	print ret

	cmd = 'adb shell su -c "cp %s/libJniTest.so %s%s/lib/arm/ "' %(transpath, apkpath, package)
	print cmd
	ret = os.popen(cmd).read()
	print ret

def clearAll(targetfile, package):
	cmd = 'del codefile'
	ret = os.popen(cmd).readlines()
	cmd = 'del dexmethod'
	ret = os.popen(cmd).readlines()
	cmd = 'del headerinfo'
	ret = os.popen(cmd).readlines()
	cmd = 'del arthack'
	ret = os.popen(cmd).readlines()
	cmd =  'del classes.dex'
	ret = os.popen(cmd).readlines()
	cmd =  'del ori_classes.dex'
	ret = os.popen(cmd).readlines()
	cmd = 'del %s' %targetfile
	ret = os.popen(cmd).readlines()
	cmd = 'del base.apk'
	ret = os.popen(cmd).readlines()
	cmd = 'del bitmap'
	ret = os.popen(cmd).readlines()
	cmd = 'del namefile'
	ret = os.popen(cmd).readlines()
	cmd = 'del notepaddump'
	ret = os.popen(cmd).readlines()
	cmd = 'rd /s/q arthack_unpack'
	ret = os.popen(cmd).readlines()
	cmd = 'rd /s/q frameworkes'
	ret = os.popen(cmd).readlines()

print "Hello, I'm artTool, you just need it"
print '########################################################'

config = ConfigParser.ConfigParser()
config.readfp(open('LG_note.ini',"rb"))
#config.readfp(open('Sony.ini',"rb"))

targetfile = config.get('global', 'targetfile')
oatpath = config.get('global', 'oatpath')
transpath = config.get('global', 'transpath')
apkpath = config.get('global', 'apkpath')
classname = config.get('global', 'classname')
password = config.get('global', 'password')

classnames = classname.split(',')

codefile = 'codefile'
package = config.get('global', 'package')
dumpfile = config.get('global', 'dumpfile')

clearAll(targetfile, package)

uninstallOri()

installApk()

if dumpfile == 'yes':
	LoadDump(oatpath, targetfile, transpath)

liblen = getOATandLib(apkpath, package, oatpath, transpath, targetfile)

getAfterApk(apkpath, package, transpath)

unpackApk()

#multi class
for name in classnames:
	method_ret, method_size = parseOat(name, password)

	dex_begin = modifyOat(targetfile, method_ret, method_size)

	method_ret = parseDex(name)

	modifyDex(targetfile, method_ret, dex_begin)
	
	insertToLib(name, password)
#end deal

repackApk(liblen)

correctCrc(targetfile)

pushOat(transpath, targetfile, oatpath)

pushApk(package, transpath)

uploadCode(transpath, codefile, apkpath, package)

print "#######################################################"