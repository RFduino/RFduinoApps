import RPi.GPIO as GPIO
import serial

GPIO.setmode(GPIO.BCM)

GPIO.setup(17, GPIO.OUT)
GPIO.setup(27, GPIO.IN)

port = serial.Serial("/dev/ttyUSB0", baudrate=9600)

def pressed(channel):
  port.write("1")
  GPIO.remove_event_detect(27)
  GPIO.add_event_detect(27, GPIO.FALLING, callback=released, bouncetime=50)

def released(channel):
  port.write("0")
  GPIO.remove_event_detect(27)
  GPIO.add_event_detect(27, GPIO.RISING, callback=pressed, bouncetime=50)

GPIO.add_event_detect(27, GPIO.RISING, callback=pressed, bouncetime=50)

loop = True
while loop:
  try:
    data = port.read()
    GPIO.output(17, data == '1');
  except KeyboardInterrupt:
    loop = False

GPIO.cleanup()
