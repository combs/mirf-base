#!/usr/bin/python

import sched, time, sys, socket, os
from paramiko import SSHClient
from PIL import ImageGrab,Image,ImageStat
from pprint import pprint
from scp import SCPClient


ourScreenWidth=ImageGrab.grab().size[0]

ourStrip=8

def millis():
	return int(round(time.time() * 1000))

def getChunk(chunk,ourImage):
	if chunk > (ourStrip-1):
		return (0,0,0,0)
	
	ourStripWidth=ourScreenWidth / (ourStrip)
	
#	return (0,0,0,1)
	ourCrop=ourImage.crop((ourStripWidth*chunk,00,ourStripWidth*(chunk+1),100))
	ourMath=ImageStat.Stat(ourCrop)
	color=(round(ourMath.mean[0]),round(ourMath.mean[1]),round(ourMath.mean[2]),round(ourMath.mean[3]))

	
	return color

def encodeColor(red,green,blue):
	encoded=encodeColorChannel(red) + encodeColorChannel(green) + encodeColorChannel(blue)
	return encoded
	
def encodeColorChannel(channel):
	channel=int(round(channel)) >> 2
	channel+=32
	return chr(channel)

s = sched.scheduler(time.time, time.sleep)

ssh = SSHClient()
ssh.load_system_host_keys()
ssh.connect("raspbmc.local",username="pi")
scp = SCPClient(ssh.get_transport())

iterator=99
enabled=1
lastMillis=millis();


def doLoop(sc) :
	global iterator, enabled, ourScreenWidth, lastMillis
	
	iterator += 1
	
	if (iterator==100):
		if (os.name=="posix"):
			if (os.system("system_profiler SPDisplaysDataType | grep -Ei 'toshiba|argley'")>0) :
				enabled=0
			else:
				enabled=1
		else:
			enabled=1
				
		ourScreenWidth=ImageGrab.grab().size[0]

	if (enabled):
				
		sc.enter(0.2, 1, doLoop, (sc,))
		colors = [0 for i in xrange(ourStrip)] 
		colorCommand="WINKYBASESA9"
		ourImage=Image
				
		try:
			ourImage=ImageGrab.grab(bbox=(0,00,ourScreenWidth,100))
				
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
			
	#		scp.put("/tmp/ambilight","/var/local/nrf24/out/ambilight")
		except socket.error as e:
			ssh.connect("raspbmc.local",username="pi")
		
#		print millis() - lastMillis
#		lastMillis=millis()


s.enter(0.2, 1, doLoop, (s,))
s.run()


	