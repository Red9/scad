import serial
import thread
import string
import time

#sudo rfcomm bind 6 00:06:66:42:1F:A0 1


delay_time = 5
transmit_done = False

def transmit_bluetooth():
	serBluetooth = serial.Serial('/dev/ttyUSB3', baudrate=230400)
	serBluetooth.flushOutput()
	for t in range(0, delay_time):
		print "Waiting to start transmission... " + str(delay_time - t)
		time.sleep(1)
	for j in range(0, 10000000):
		for i in range(0, len(string.lowercase)):
			serBluetooth.write(string.lowercase[i])
			time.sleep(0.0001)
	print "Closing: "
	serBluetooth.close()
	print "Done closing."
	transmit_done = True






thread.start_new_thread(transmit_bluetooth, ())
#time.sleep(delay_time - 1);
serUSB = serial.Serial('/dev/ttyUSB4', baudrate=460800)
serUSB.flushInput()

predicted_letter = ord('a');

letter_count = 0
failure_count = 0.0
last_mismatch = 0

while transmit_done == False:
	usb_letter = ord(serUSB.read(1)[0])
	letter_count = letter_count + 1
	if(letter_count == 8000):
		failure_count = 0 #Hack to reset since the first ~5000 lines are bad for some reason
	
	if(usb_letter != predicted_letter):
		failure_count = failure_count + 1.0
		print "Mismatch at letter " + str(letter_count) + ": " \
			+ chr(predicted_letter) + " != " + chr(usb_letter) \
			+ " delta: " + str(letter_count - last_mismatch)  \
			+ "\tfailure rate: " + str((failure_count / letter_count) * 100.0) + "%"
		predicted_letter = usb_letter
		last_mismatch = letter_count
		
	
	predicted_letter = predicted_letter + 1
	if(predicted_letter > ord('z')):
		predicted_letter = ord('a')

serUSB.close()


#

#serBluetooth.write("Hello, Bluetooth!")

#byte = serUSB.read()
#for i in range(100):
#	print i, byte
#	byte = serUSB.read()



#
#serBluetooth.close();

