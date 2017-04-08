#!/usr/bin/env python
# -*- coding: utf-8 -*-

import network
from simple import MQTTClient
from machine import Pin, I2C
import utime

# Received messages from subscriptions will be delivered to this callback
def sub_cb(topic, msg):
	if msg == b"1": 
		# construct an I2C bus
		i2c = I2C(scl=Pin(5), sda=Pin(4), freq=100000)
		# send Sony power message
		send(0xa90, i2c)
		
def make_connection():
	while 1:
		try:
			print('trying to reconnect')
			if not c.connect(clean_session=False):
				print("New session being set up")
				c.subscribe(b"in/livingroom/tv")
			return
		except OSError as e:
			print ('Continued connection fail', e)
			utime.sleep(1)

def send(val, i2c):
	""" Sends a value to the ATTiny85 IR Transmitter, which transmits the code
	in Sony 12 bit format """
	b= bytearray(1)				# i2c takes a buffer
	
	b[0] = 0xff					# load the buffer with the reset byte
	acks = i2c.writeto(0x26, b)	# write it out to the transmitter
	utime.sleep_ms(100)			# wait a bit
	
	b[0] = val >> 8				# load the buffer with the top 8 bits of the 16 bit value
	acks = i2c.writeto(0x26, b)	# write it out to the transmitter
	utime.sleep_ms(100)			# wait a bit
	
	b[0] = val & 0xff			# load the buffer with the bottom 8 bits of the 16 bit value
	acks = i2c.writeto(0x26, b)	# write it out to the transmitter
	utime.sleep_ms(100)			# wait a bit
	
	c = bytearray(2)			# make two byte read buffer
	try:
		i2c.readfrom_into(0x26, c)	# request two byte from the transmitter
	except Exception as e:
		print ('I2C read error: ', e)
		i2c.writeto(0x26, 'x')		# send a sync byte
	print('printing result')
	result = (c[0] << 8) | c[1]
	print ('{:04x}'.format(result))	# print the result

	
pwr = Pin(0, Pin.IN)

sta_if = network.WLAN(network.STA_IF)
while not sta_if.isconnected():
	print("waiting for network")
	utime.sleep(1)

print("Starting")
	
c = MQTTClient("tv", "iot_server")
c.set_callback(sub_cb)

make_connection()

def main ():
	while True:
		try:
			c.check_msg()
		except OSError as e:
			print ('Initial connection fail', e)
			make_connection()
			
		print('Power: ' + str(pwr.value()))
		if pwr.value()==1:
			message = '1';
		else:
			message = '0'
		try:
			c.publish(b"out/livingroom/tv", bytearray(message))
		except OSError as e:
			print ('Publish fail ' + e)
			# subscription will also fail and make reconnection

		utime.sleep(1)
	
c.disconnect()
