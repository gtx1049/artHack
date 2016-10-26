#dump file deal

import sys
import os

def LoadDump(oatpath, targetfile, transpath):

	cmd = 'adb shell su -c "oatdump --oat-file=%s%s --output=%soatdump"' %(oatpath, targetfile, transpath)
	print cmd
	print " dumping oat file ... Please Wait ..."
	ret = os.popen(cmd).read()
	print ret

	cmd = 'adb pull %soatdump' %transpath
	print cmd
	print "pulling dump file ... Please Wait ..."
	ret = os.popen(cmd).read()
	print ret
	return
	
def findIndump(modi, offset, codesize, index):
	
	filein = 'oatdump'
	
	try:
		fdin = open(filein,'r')
		namefile = open('namefile', 'r')
	except IOError:
		print 'open %s failed!!!',filein
		sys.exit(1)
	
	search = namefile.readline();
	search = ': ' + search.strip()
	print search
	
	last = 0
	lastline = ''
	
	index_s = str(index)
	
	for line in fdin:
		if line.find(search) != -1:
			print 'find'
			for innerline in fdin:
				innerline = innerline.strip()
				m = innerline.find(':')
				if m != -1:
					lineindex = innerline[0 : m + 1]
					if lineindex == (index_s + ':'):
						#print innerline
						for code in fdin:
							if code.find('OatMethodOffsets') != -1:
								break
							if code.find('DEX CODE') != -1:
								continue
							pos_s = code[code.find('0x') : code.index(':')]
							pos_i = int(pos_s, 16);
							
							#print str(pos_i) + ' : ' + str(last) + ' : ' + str(pos_i - last)
							
							if pos_i - last > 0 and lastline.find('invoke-') == -1:
								#print lastline
								#print 'delete '
								modi.seek(offset + last * 2)
								
								#print str(offset + last) + ':::' + str(pos_i - last)
								
								#for i in range(0, (pos_i - last) * 2):
									#modi.write(b'\x00')
								for i in range(0, (pos_i - last)):
									modi.write(b'\x01\x10')
								
								
							last = pos_i
							lastline = code
						break
			break		