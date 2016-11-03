import sys
import os
import ConfigParser

def makeDiff():

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

	commandOne = 'adb shell su -c "cp %s%s %s"' %(oatpath, targetfile, transpath)
	commandTwo = 'adb pull %s%s' %(transpath, targetfile)
	commandThree = 'copy %s arthack.shadow' %targetfile

	print commandOne
	print commandTwo
	print commandThree

	ret = os.popen(commandOne).read()
	print ret
	ret = os.popen(commandTwo).read()
	print ret

	ret = os.popen(commandThree).read()
	print ret
	
	cmd = 'adb shell su -c "rm /data/dalvik-cache/arm/system@framework@boot.art"'
	ret = os.popen(cmd).read()
	print ret
	
	cmd = 'adb reboot'
	ret = os.popen(cmd).read()
	print ret
	
makeDiff()