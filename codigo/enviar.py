#! /usr/bin/python
import serial
import cmd

Dato_Temp = 'T'
Dato_Temp_Amb = 'A'
Modo_Frio = 'F'
Modo_Calor = 'C'
Modo_Apagado = 'X'
Error = 'E'

class EnviarDatos(cmd.Cmd):
	def do_temperatura(self, dato): 
		""" Modo de uso: temperatura [temperatura]
		Envia la temperatura que se desea en la celda peltier. La temperatura debe un numero entre -10 C y 56 C """
		temp = int(dato)
		if (temp < -10 or temp > 56):
			print "Temperatura invalida. La temperatura debe valer entre -10 C y 56 C."
		else:
			serial_port = serial.Serial("/dev/ttyUSB0",38400)
			serial_port.write('T')
			serial_port.write(chr(temp))
			serial_port.close()
	
	def do_frio(self, dato): 
		""" Modo de uso: frio [pwm]
		Establece el modo de operacion frio con el pwm ingresado. El pwm controla la corriente que circula por la celda peltier, debe
		ser un numero entre 0 y 255 """
		pwm = int(dato)
		if (pwm < 0 or pwm > 255):
			print "pwm invalido. Son validos los valores entre 0 y 255."
		else:
			serial_port = serial.Serial("/dev/ttyUSB0",38400)
			serial_port.write('F')
			serial_port.write(chr(pwm))
			serial_port.close()

	def do_calor(self, dato): 
		""" Modo de uso: frio [pwm]
		Establece el modo de operacion calor con el pwm ingresado. El pwm controla la corriente que circula por la celda peltier, debe
		ser un numero entre 0 y 255 """
		pwm = int(dato)
		if (pwm < 0 or pwm > 255):
			print "pwm invalido. Son validos los valores entre 0 y 255."
		else:
			serial_port = serial.Serial("/dev/ttyUSB0",38400)
			serial_port.write('C')
			serial_port.write(chr(pwm))
			serial_port.close()
			
	def apagar(self, dato): 
		""" Modo de uso: apagar
		Anula la circulacion de corriente en la celda peltier """
		serial_port = serial.Serial("/dev/ttyUSB0",38400)
		serial_port.write('X')
		serial_port.close()


if __name__ == '__main__':
    EnviarDatos().cmdloop()
