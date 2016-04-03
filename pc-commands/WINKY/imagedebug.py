#!/usr/bin/python

import sched, time, sys, socket, os
from paramiko import SSHClient
from PIL import ImageGrab,Image,ImageStat
from pprint import pprint
from scp import SCPClient


ourScreenWidth=ImageGrab.grab().size[0]
ourOffset=0
ourStrip=8

def millis():
	return int(round(time.time() * 1000))

def getChunk(chunk,ourImage):
	global ourOffset
	if chunk > (ourStrip-1):
		return (0,0,0,0)
	
	ourStripWidth=ourScreenWidth / (ourStrip)
	
	ourCrop=ourImage.crop((ourStripWidth*chunk,0,ourStripWidth*(chunk+1),100))
	ourMath=ImageStat.Stat(ourCrop)
	color=(round(ourMath.rms[0]),round(ourMath.rms[1]),round(ourMath.rms[2]),round(ourMath.rms[3]))
	if (color[0]<9 and color[1]<9 and color[2] < 9 and color[0]>0 and color[1]>0 and color[2]>0):
		color[0]=color[1]=color[2] 
#	pprint((ourStripWidth*chunk,0,ourStripWidth*(chunk+1),100));
#	pprint(color);
	
	return color

def encodeColor(red,green,blue):
	encoded=encodeColorChannel(red) + encodeColorChannel(green) + encodeColorChannel(blue)
	return encoded
	
def encodeColorChannel(channel):
	channel=int(round(channel)) >> 2
	channel+=32
	return chr(channel)

def getOffset(ourImage):
	global ourOffset
	ourX=int(round(ourImage.size[0]/2))
	for chunk in range(40):
		ourCrop=ourImage.crop((ourX,(chunk-1)*10,ourX+1,chunk*10)) 
		ourMath=ImageStat.Stat(ourCrop)
		color=(round(ourMath.rms[0]),round(ourMath.rms[1]),round(ourMath.rms[2]),round(ourMath.rms[3]))
		if (color[0]>0 or color[1]>0 or color[2]>0):
			ourOffset=(chunk-1)*10
			break

s = sched.scheduler(time.time, time.sleep)

ssh = SSHClient()
ssh.load_system_host_keys()
ssh.connect("raspbmc.local",username="pi")
scp = SCPClient(ssh.get_transport())

iterator=99
enabled=1
lastMillis=millis();


def doLoop(sc) :
	global iterator, enabled, ourScreenWidth, lastMillis, ourOffset
	
	iterator += 1
	
	sc.enter(0.2, 1, doLoop, (sc,))
	
	if (iterator==100):
	
		if (enabled):
		
			print "checking monitor..."
			iterator=0
			if (os.name=="posix"):
				if (os.system("system_profiler SPDisplaysDataType | grep -Ei 'toshiba|argley'")>0) :
					enabled=1
					print "TOSHIBA or ARGLEY not found. Sleeping..."
				else:
					enabled=1
					print "screen connected."
			else:
				enabled=1
			bigImage=Image.open(sys.argv[1]).crop((0,00,ourScreenWidth,440))
			ourScreenWidth=bigImage.size[0]
			getOffset(bigImage)
			print "letterbox offset: " + str(ourOffset)
			
		else:
		
			bigImage=ImageGrab.grab(bbox=(0,00,ourScreenWidth,440))
			getOffset(bigImage)
			print(ourOffset)
			
			print "sleeping..."

	if (enabled):
		
		colors = [0 for i in xrange(ourStrip)] 
		colorCommand="WINKYBASESA9"

		try:
			ourImage=Image.open(sys.argv[1]).crop((0,ourOffset,ourScreenWidth,ourOffset+100))
				
			for chunk in reversed(range(len(colors))):
				colors[chunk]=getChunk(chunk,ourImage)
				if colors[chunk][3]==0:
					print "screen width changed, got transparent area"
					ourWidth=ImageGrab.grab().size[0]
					colors[chunk]=getChunk(chunk)
				colorCommand=colorCommand + encodeColor(colors[chunk][0],colors[chunk][1],colors[chunk][2])
		#	colorCommand=colorCommand + "\n\r"

		except:
			print "capture yacked"
			#	fileOut = open("/tmp/ambilight","w+")
	#	fileOut.write(colorCommand)
	#	fileOut.close()
	
		colorCommand = colorCommand.replace("\"", r"\"")
#	 	print "echo \"" + colorCommand + "\">/var/local/nrf24/out/ambilight"
	 	
		try: 
			ssh.exec_command("echo \"" + colorCommand + "\">/var/local/nrf24/out/ambilight") 
			# print str(iterator)+" " + str(ourOffset)
			
			
	#		scp.put("/tmp/ambilight","/var/local/nrf24/out/ambilight")
		except socket.error as e:
			print "connection failed"
			ssh.connect("raspbmc.local",username="pi")
		
#		print millis() - lastMillis
#		lastMillis=millis() 	

print "starting up task..."
s.enter(0.2, 1, doLoop, (s,))
s.run()
