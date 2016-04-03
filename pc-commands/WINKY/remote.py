import serial,time,colorsys,socket,glob

tags = { 
	"".join( [ chr(128) , chr(128) ] ): 'BUTTON_CENTER', 
	"".join( [ chr(48) , chr(48) ] ): 'BUTTON_N', 
	"".join( [ chr(49) , chr(49) ] ): 'BUTTON_NNE', 
	"".join( [ chr(50) , chr(50) ] ): 'BUTTON_NE', 
	"".join( [ chr(51) , chr(51) ] ): 'BUTTON_ENE', 
	"".join( [ chr(52) , chr(52) ] ): 'BUTTON_E', 
	"".join( [ chr(53) , chr(53) ] ): 'BUTTON_ESE', 
	"".join( [ chr(54) , chr(54) ] ): 'BUTTON_SE', 
	"".join( [ chr(55) , chr(55) ] ): 'BUTTON_SSE', 
	"".join( [ chr(56) , chr(56) ] ): 'BUTTON_S', 
	"".join( [ chr(57) , chr(57) ] ): 'BUTTON_SSW', 
	"".join( [ chr(58) , chr(58) ] ): 'BUTTON_SW', 
	"".join( [ chr(59) , chr(59) ] ): 'BUTTON_WSW', 
	"".join( [ chr(60) , chr(60) ] ): 'BUTTON_W',
	"".join( [ chr(61) , chr(61) ] ): 'BUTTON_WNW',
	"".join( [ chr(62) , chr(62) ] ): 'BUTTON_NW',
	"".join( [ chr(63) , chr(63) ] ): 'BUTTON_NNW',
	"".join( [ chr(222) , chr(173) ] ): 'POWER_SLEEP',
	"".join( [ chr(190) , chr(239) ] ): 'POWER_WAKE',
	"".join( [ chr(252) , chr(32) ] ): 'GESTURE_START_N', 
	"".join( [ chr(252) , chr(33) ] ): 'GESTURE_START_NNE', 
	"".join( [ chr(252) , chr(34) ] ): 'GESTURE_START_NE', 
	"".join( [ chr(252) , chr(35) ] ): 'GESTURE_START_ENE', 
	"".join( [ chr(252) , chr(36) ] ): 'GESTURE_START_E', 
	"".join( [ chr(252) , chr(37) ] ): 'GESTURE_START_ESE', 
	"".join( [ chr(252) , chr(38) ] ): 'GESTURE_START_SE', 
	"".join( [ chr(252) , chr(39) ] ): 'GESTURE_START_SSE', 
	"".join( [ chr(252) , chr(40) ] ): 'GESTURE_START_S', 
	"".join( [ chr(252) , chr(41) ] ): 'GESTURE_START_SSW', 
	"".join( [ chr(252) , chr(42) ] ): 'GESTURE_START_SW', 
	"".join( [ chr(252) , chr(43) ] ): 'GESTURE_START_WSW', 
	"".join( [ chr(252) , chr(44) ] ): 'GESTURE_START_W',
	"".join( [ chr(252) , chr(45) ] ): 'GESTURE_START_WNW',
	"".join( [ chr(252) , chr(46) ] ): 'GESTURE_START_NW',
	"".join( [ chr(252) , chr(47) ] ): 'GESTURE_START_NNW',
	"".join( [ chr(251) , chr(32) ] ): 'GESTURE_STOP_N', 
	"".join( [ chr(251) , chr(33) ] ): 'GESTURE_STOP_NNE', 
	"".join( [ chr(251) , chr(34) ] ): 'GESTURE_STOP_NE', 
	"".join( [ chr(251) , chr(35) ] ): 'GESTURE_STOP_ENE', 
	"".join( [ chr(251) , chr(36) ] ): 'GESTURE_STOP_E', 
	"".join( [ chr(251) , chr(37) ] ): 'GESTURE_STOP_ESE', 
	"".join( [ chr(251) , chr(38) ] ): 'GESTURE_STOP_SE', 
	"".join( [ chr(251) , chr(39) ] ): 'GESTURE_STOP_SSE', 
	"".join( [ chr(251) , chr(40) ] ): 'GESTURE_STOP_S', 
	"".join( [ chr(251) , chr(41) ] ): 'GESTURE_STOP_SSW', 
	"".join( [ chr(251) , chr(42) ] ): 'GESTURE_STOP_SW', 
	"".join( [ chr(251) , chr(43) ] ): 'GESTURE_STOP_WSW', 
	"".join( [ chr(251) , chr(44) ] ): 'GESTURE_STOP_W',
	"".join( [ chr(251) , chr(45) ] ): 'GESTURE_STOP_WNW',
	"".join( [ chr(251) , chr(46) ] ): 'GESTURE_STOP_NW',
	"".join( [ chr(251) , chr(47) ] ): 'GESTURE_STOP_NNW',
	"".join( [ chr(251) , chr(251) ] ): 'GESTURE_STOP'
	
	} 
categories = {
	
	chr(1): 'GESTURE_CW_SLOW',
	chr(2): 'GESTURE_CW_MEDIUM',
	chr(3): 'GESTURE_CW_FAST',
	chr(4): 'GESTURE_CW_SUPERFAST',
	chr(5): 'GESTURE_CW_SUPERFAST',
	chr(6): 'GESTURE_CW_SUPERFAST',
	chr(7): 'GESTURE_CW_SUPERFAST',
	chr(17): 'GESTURE_CCW_SLOW',
	chr(18): 'GESTURE_CCW_MEDIUM',
	chr(19): 'GESTURE_CCW_FAST',
	chr(20): 'GESTURE_CCW_SUPERFAST',
	chr(21): 'GESTURE_CCW_SUPERFAST',
	chr(22): 'GESTURE_CCW_SUPERFAST',
	chr(23): 'GESTURE_CCW_SUPERFAST'
	
}


def encodeColor(red,green,blue):
	encoded=encodeColorChannel(red) + encodeColorChannel(green) + encodeColorChannel(blue)
	return encoded
	
def encodeColorChannel(channel):
	channel=int(round(channel)) >> 2
	channel+=32
	return chr(channel)


def findPort():
	temp_list = glob.glob ('/dev/*uart-[A-Za-z0-9]*')
	result = []
	for a_port in temp_list:
		print "trying " + a_port
		try:
			s = serial.Serial (a_port)
			s.close ()
			print "good"
			result.append (a_port)
		except Exception:
			print "yacked"
			pass
	print result
	return result

theports=findPort();
ser = serial.Serial()
if len(theports) > 0 :
	try:
		ser = serial.Serial(theports[0])
	except serial.SerialException as e:
		print "could not open port:"
		print e	
else:
	print "no ports available at /dev/*uart-*"

lasttime=time.time();
provisionaltime=time.time();
gesturestarted=False
gesturestopping=False
lastparsed='POWER_WAKE'

currentColor=[0,0,0]

TCP_IP = "raspbmc.local"
TCP_PORT = 10000
BUFFER_SIZE=1024

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((TCP_IP, TCP_PORT))


while 1:
	thistime = time.time()
	try:
		x = ser.read(2)
	except serial.SerialException as e:
		x = "".join( [ chr(128) , chr(128) ] )
		print "port closed:"
		print e
		theports=findPort();
		if len(theports) > 0 :
			ser = serial.Serial(theports[0])
		else:
			print "no ports available at /dev/*uart-*, sleeping"
			time.sleep(15)
			
	send = False
	parsed=", ".join( [ str(ord(x[0])) , str(ord(x[1])) ] )
	if x in tags:
		parsed = tags[x]
	elif x[0] in categories:
		parsed = categories[x[0]]
		
	if 'GESTURE_START' in parsed:
		gesturestarted=True
		gesturestopping=False
		print 'gesture started...'
	elif 'GESTURE_STOP' in parsed:
		provisionaltime=time.time()
		gesturestopping=True
		print 'gesture stopping...'

	if (gesturestopping == True) and ((thistime - provisionaltime)*1000 > 500):
		gesturestarted=False
		gesturestopping=False
		print 'gesture stopped'
	
	if gesturestarted==True:
		if 'GESTURE' not in parsed:
			parsed='IGNORED'
	
	
	elif lastparsed==parsed and ((thistime - lasttime)*1000 < 300):
		parsed='DEBOUNCED'
		
	print parsed + " " + str(round((thistime - lasttime)*1000))

	if 'BUTTON_N' in parsed:
		if (currentColor[2] < 1.0):
			currentColor[2] = currentColor[2] + 0.1
		if (currentColor[2] > 1.0) :
			currentColor[2] = 1.0
		if (currentColor[2] < 0) :
			currentColor[2] = 0
	elif 'BUTTON_S' in parsed:
		if (currentColor[2] > 0.1):
			currentColor[2] = currentColor[2] - 0.1
	elif 'BUTTON_W' in parsed:
		if (currentColor[1] > 0.25):
			currentColor[1] = currentColor[1] - 0.25
		else:
			currentColor[1] = 0
	elif 'BUTTON_E' in parsed:
		if (currentColor[1] < 1.0):
			currentColor[1] = currentColor[1] + 0.25
		
	elif 'BUTTON_CENTER' in parsed:
		if (currentColor[2] > 0.0):
			currentColor[2] = 0
		else:
			currentColor[2] = 1
	elif 'GESTURE_CW' in parsed:
		if (currentColor[0] > 0.025):
			currentColor[0] = currentColor[0] - 0.025
		else:
			currentColor[0] = 1.0
	elif 'GESTURE_CCW' in parsed:
		if (currentColor[0] < 0.975):
			currentColor[0] = currentColor[0] + 0.025
		else:
			currentColor[0] = 0
	
	ourhsv = list(colorsys.hsv_to_rgb(currentColor[0],currentColor[1],currentColor[2]))
	ourhsv[0] = round(ourhsv[0] * 255)
	ourhsv[1] = round(ourhsv[1] * 255)
	ourhsv[2] = round(ourhsv[2] * 255)

	if 'GESTURE' in parsed or 'BUTTON' in parsed:
		send = True
		
	if send == True:

		encodedColor = encodeColor(ourhsv[0],ourhsv[1],ourhsv[2])
		colorCommand="WINKYBASESX9" + encodedColor + encodedColor + "\r\n"

	 	try: 
			print(colorCommand)
			sock.send(colorCommand)
		except Exception as e:
			print "connection failed"
			sock.connect((TCP_IP, TCP_PORT))


	lasttime=thistime
	lastparsed=parsed