#! /usr/bin/python
import serial
from time import time

TAMANIO_TEMPERATURA = 1

VOLT = 2.63 / 255

def traducir(leido):
	return ord(leido)

def traducir_caracter(leido):
	return chr(ord(leido))

def traducir_temperatura(leido):
	unsigned = ord(leido)
	signed = unsigned - 256 if unsigned > 127 else unsigned
	return signed
	

DATA_TYPE = {"A":("Temperatura Ambiente",TAMANIO_TEMPERATURA,traducir_temperatura),
		"T":("Temperatura Peltier",TAMANIO_TEMPERATURA,traducir_temperatura),
		"D":("Caracter recibido",TAMANIO_TEMPERATURA,traducir_caracter),}

def main():
	serial_port = serial.Serial("/dev/ttyUSB0",38400)
	while (True): #Leo el tipo de dato a leer
		read_byte = serial_port.read(1)	
		if not DATA_TYPE.has_key(read_byte):
			print "Leido: ",ord(read_byte), "no reconocido."
			continue

		data_type,data_len,f = DATA_TYPE[read_byte]
		read_data = serial_port.read(data_len)
		if not read_data:
			print "Fallo lectura"
			break

		print data_type, " : " , f(read_data)

	serial_port.close()
main()
