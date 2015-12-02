#!/usr/bin/python

import sched, time, sys, socket, os
from PIL import ImageGrab,Image,ImageStat
from pprint import pprint


ourScreenWidth=ImageGrab.grab().size[0]
ourOffset=0
ourStrip=8
TCP_IP = "raspbmc.local"
TCP_PORT = 10000
BUFFER_SIZE=1024


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
		color[0]=color[1];
		color[2]=color[1]; 
	
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
	print "sampling at X: " +str(ourX)
	for chunk in range(1,40):
		ourCrop=ourImage.crop((ourX,(chunk-1)*10,ourX+1,chunk*10)) 
		ourMath=ImageStat.Stat(ourCrop)
		pprint(ourMath.rms)
		color=( round(ourMath.rms[0]),round(ourMath.rms[1]),round(ourMath.rms[2]),round(ourMath.rms[3]) )
		if (color[0]>0 or color[1]>0 or color[2]>0):
			ourOffset=(chunk-1)*10
			break

s = sched.scheduler(time.time, time.sleep)

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((TCP_IP, TCP_PORT))
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
					enabled=0
					print "TOSHIBA or ARGLEY not found. Sleeping..."
				else:
					enabled=1
					print "screen connected."
			else:
				enabled=1
					
			bigImage=ImageGrab.grab(bbox=(0,0,ourScreenWidth,440))
			ourScreenWidth=bigImage.size[0]
			print "screen size: " + str(ourScreenWidth)
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
			ourImage=ImageGrab.grab(bbox=(0,ourOffset,ourScreenWidth,ourOffset+100))
				
			for chunk in reversed(range(len(colors))):
				colors[chunk]=getChunk(chunk,ourImage)
				if colors[chunk][3]==0:
					print "screen width changed, got transparent area"
					ourWidth=ImageGrab.grab().size[0]
					colors[chunk]=getChunk(chunk)
				colorCommand=colorCommand + encodeColor(colors[chunk][0],colors[chunk][1],colors[chunk][2])

		except:
			print "capture yacked: ",sys.exc_info()[0]


			#	fileOut = open("/tmp/ambilight","w+")
	#	fileOut.write(colorCommand)
	#	fileOut.close()
	
		colorCommand = colorCommand.replace("\"", r"\"") + "\r\n"

#	 	print "echo \"" + colorCommand + "\">/var/local/nrf24/out/ambilight"
	 	try: 
			print(colorCommand)
			sock.send(colorCommand)
			# print str(iterator)+" " + str(ourOffset)
			
			
	#		scp.put("/tmp/ambilight","/var/local/nrf24/out/ambilight")
		except socket.error as e:
			print "connection failed"
			sock.connect((TCP_IP, TCP_PORT))
		
#		print millis() - lastMillis
#		lastMillis=millis() 	

print "starting up task..."
s.enter(0.2, 1, doLoop, (s,))
s.run()
