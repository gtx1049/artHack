import sys
import os
import time
import struct
import zlib
from ctypes import *

def clear():
	cmd = 'rd /s/q arthack_unpack_install'
	ret = os.popen(cmd).readlines()
	cmd = 'rd /s/q frameworkes_install'
	ret = os.popen(cmd).readlines()

def unpackInstallApk():
	cmd = 'apktool d base.apk -p frameworkes_install -f -o arthack_unpack_install -s'
	#cmd = 'apktool d base.apk -p frameworkes_install -f -o arthack_unpack_install'
	print cmd
	ret = os.popen(cmd).read()
	print ret
	return
	
def repackInstallApk():

	cmd = 'copy /y libJniTest.so arthack_unpack_install\\lib\\armeabi\\libJniTest.so'
	ret = os.popen(cmd).read()
	print ret
	
	cmd = 'copy /y  classes.dex arthack_unpack_install\classes.dex'
	ret = os.popen(cmd).read()
	print ret
	
	checksum()
	
	cmd = 'apktool b arthack_unpack_install -p frameworkes_install'
	ret = os.popen(cmd).read()
	print ret
	
	# sign the apk
	cmd = "java -jar sign.jar .\\arthack_unpack_install\\dist\\base.apk --override"
	ret = os.popen(cmd).read()
	print ret
	
	return

def checksum():

	f = open('.\\arthack_unpack_install\\classes.dex',"r+b")
	f.seek(12)
	ucrc = zlib.adler32(f.read()) 
	if ucrc>0:
		print ">>>>> " + str(ucrc)
		uoutint=ucrc
	else :
		print "<<<<< " + str(ucrc)
		uoutint= ~ ucrc ^ 0xffffffff
	print str(uoutint)
	byte = struct.pack('I', uoutint)
	f.seek(8)
	f.write(byte)
	f.close()
	
clear()
unpackInstallApk()
repackInstallApk()