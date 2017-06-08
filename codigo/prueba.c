#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#define F_CPU 8000000UL
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "prueba.h"

#define Dato_Temp 	'T'
#define Dato_Temp_Amb 	'A'
#define Modo_Frio 	'F'
#define Modo_Calor 	'C'
#define Modo_Apagado	'X'
#define Modo_Pulso_Frio 'P'
#define Modo_Pulso_Calor 'Q'
#define Dato_Char	'D'
#define PWM_Max 110
#define PWM_Calor 70


#define LEN 67

volatile int8_t Tinicial;
volatile uint8_t PWM_fino = 0;

static const uint8_t VOLT[LEN] PROGMEM =
{247, 240, 234, 227, 221, 214, 208, 202, 196, 190, 184,
178, 172, 166, 161, 155, 150, 145, 140, 135, 131, 126,
122, 117, 113, 109, 105, 101, 98, 94, 91, 87, 84, 81, 78,
76, 74, 71, 68, 66, 63, 61, 59, 57, 54, 52, 51, 49, 47,
45, 44, 42, 40, 39, 38, 36, 35, 34, 33, 31, 30, 29, 28,
27, 26, 25, 24};


static const int8_t TEMP[LEN] PROGMEM =
{-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,
10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,
28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,
46,47,48,49,50,51,52,53,54,55,56
};


/* ******************************************************************* *
 * 						 			USART                              *
 * ******************************************************************* */

void USART_Init(void){
	UCSRB = (1<<TXEN) | (1<<RXEN) | (1<<RXCIE); /* Transmit y Receive con interrupcion */
	UCSRC = (1<<URSEL)|(3<<UCSZ0); /* 8 data, 1 stop bit */
	UBRRL = 0xC;
}

void transmit(int8_t dato){
	while( !(UCSRA&(1<<UDRE)) ){}
	UDR = dato;
}

int8_t receive(void){
	while ( !(UCSRA & (1<<RXC)) ){}
	return UDR;
}

ISR(USART_RXC_vect){
	char dato = receive();
	
	transmit(Dato_Char);
	transmit(dato);
	
	
	switch(dato){
		case(Dato_Temp):
			dato = receive();
			sei();
			loop_peltier(dato);
		case(Modo_Calor):
			dato = receive();
			sei();
			PWM_set(dato);
			modo_calor();
			loop_transmit();
		case(Modo_Frio):
			dato = receive();
			sei();
			PWM_set(dato);
			modo_frio();
			loop_transmit();
		case(Modo_Apagado):
			PWM_off();
			loop_transmit();
		case(Modo_Pulso_Frio):
			dato = receive();
			sei();
			PWM_set(dato);
			modo_frio();
			_delay_ms(1000);
			PWM_off();
			loop_transmit();
		case(Modo_Pulso_Calor):
			dato = receive();
			sei();
			PWM_set(dato);
			modo_calor();
			_delay_ms(1000);
			PWM_off();
			loop_transmit();
		default:
			while(1);
	}
	
}

/* ******************************************************************* *
 * 						 			ADC	                               *
 * ******************************************************************* */

void ADC_Init(void){
	DDRC = 0;
	ADCSRA = 0x87;
}

int8_t obtener_Tamb(void){
	ADMUX = 0xE3;
	ADCSRA |= (1<<ADSC);
	while ( (ADCSRA & (1<<ADIF)) == 0 ){}
	ADCSRA |= (1<<ADIF);
	return traducir_temp(ADCH);
}

int8_t obtener_Tpeltier(void){
	ADMUX = 0xE2;
	ADCSRA |= (1<<ADSC);
	while ( (ADCSRA & (1<<ADIF)) == 0 ){}
	ADCSRA |= (1<<ADIF);
	return traducir_temp(ADCH);
}

/* ******************************************************************* *
 * 						 			PWM	                               *
 * ******************************************************************* */

void PWM_Init(void){
	TCCR2 = 0x71;
}

void PWM_on(void){
	DDRB |= (1<<3);	
}

void PWM_off(void){
	DDRB &= ~(1 << 3);
	PWM_set(0);
}

void PWM_set(uint8_t pwm){
	OCR2 = 255-pwm;
	PWM_on();
}

/* ******************************************************************* *
 * 						 PROGRAMA PRINCIPAL                            *
 * ******************************************************************* */

int main(void){
	int8_t T;

	USART_Init();
	ADC_Init();
	PWM_Init();
	
	T = obtener_Tamb();
	transmit(Dato_Temp_Amb);
	transmit(T);
	
	T = obtener_Tpeltier();
	transmit(Dato_Temp);
	transmit(T);
	Tinicial = T;
	
	sei();
	while(1);
	
	return 0;
}


void loop_transmit(void){
	int8_t T;	
	while(1){
		T = obtener_Tpeltier();
		transmit(Dato_Temp);
		transmit(T);
		_delay_ms(250);
	}
}



void loop_peltier(int8_t Tdeseada){
	int8_t T, dif;
	char modo = obtener_modo(Tdeseada);
	PWM_fino = 50;
	if (modo == Modo_Calor) PWM_fino = 15;
	
	while (1){
		T = obtener_Tpeltier();
		transmit(Dato_Temp);
		transmit(T);
		
		dif = T-Tdeseada;
		
		if (abs(dif) > 1) regular_grueso(dif);
		else regular_fino(Tdeseada, dif);
		
		_delay_ms(500);
	}
}

void regular_grueso(int8_t dif){
	if (dif > 0){
		modo_frio();
		PWM_set(PWM_Max);
	}else if (dif < 0){
		modo_calor();
		PWM_set(PWM_Calor);
	}else{
		PWM_off();
	}
}

void regular_fino(int8_t Tdeseada, int8_t dif){
	char modo = establecer_modo(Tdeseada);
	PWM_set(PWM_fino);
	
	if (modo == Modo_Frio){
		if(dif < 0){
			if (PWM_fino >= 25) PWM_fino -= 5;
			else PWM_fino = 20;
		}else if(dif > 0){
			PWM_fino += 5;
			if (PWM_fino > PWM_Max) PWM_fino = PWM_Max; 
		}
	}else if(modo == Modo_Calor){
		if(dif > 0){
			if (PWM_fino >= 5) PWM_fino -= 2;
			else PWM_fino = 0;
		}else if(dif < 0){
			PWM_fino += 2;
			if (PWM_fino > 50) PWM_fino = 50; 
		}

	}else{
		PWM_fino = 0;
	}
	
}

/*
			if (Tpeltier - Tdeseada < 5){ 
				pwm = 20;
			}else if (Tpeltier - Tdeseada < 10){ 
				pwm = 40;
			}else if (Tpeltier - Tdeseada < 15){ 
				pwm = 60;
			}else if (Tpeltier - Tdeseada < 20){ 
				pwm = 80;
			}else{
				pwm = 100;
			} 
			
			while (abs(dif) <= 1){
				T = obtener_Tpeltier();
				transmit(Dato_Temp_Amb);
				transmit(T);
				dif = T-Tdeseada;
				
				if(dif == -1){
					if (pwm > 25) pwm -= 5;
					else pwm = 20;
					
				}else if(dif == 1){
					pwm += 5;
					if (Tpeltier - Tdeseada > 15 && pwm > PWM_Max){
						pwm = PWM_Max; 
					}else if (pwm > 70){
						 pwm = 70;
					}
				}
				PWM_set(pwm);
				_delay_ms(1000);
			}
		
}*/

char obtener_modo(int8_t Tdeseada){
	if (Tdeseada == Tinicial){
		PWM_off();
		return Modo_Apagado;
	}
	PWM_set(0);
	if (Tdeseada < Tinicial){
		return Modo_Frio;
	}
	return Modo_Calor;
}

void aplicar_modo(char modo){
	if (modo == Modo_Calor) modo_calor();
	else if (modo == Modo_Frio) modo_frio();
}

char establecer_modo(int8_t Tdeseada){
	char modo = obtener_modo(Tdeseada);
	if (modo == Modo_Calor) modo_calor();
	else if (modo == Modo_Frio) modo_frio();
	return modo;
}

void modo_calor(void){
	DDRD |= (3 << 6);
	PORTD &= ~(3 << 6);
	PORTD |= (1 << 6);
}

void modo_frio(void){
	DDRD |= (3 << 6);
	PORTD &= ~(3 << 6);
	PORTD |= (2 << 6);
}

int8_t traducir_temp(uint8_t volt){
	uint8_t volt1, volt2;
	uint8_t n = 0;
	
	while (volt < pgm_read_byte(&(VOLT[n]))){
		n++;
	}
	
	volt1 = pgm_read_byte(&(VOLT[n]));
	volt2 = pgm_read_byte(&(VOLT[n-1]));
	volt1 = volt - volt1;
	volt2 = volt2 - volt;
	if (volt1 > volt2) n--;
	
	return pgm_read_byte(&(TEMP[n]));
}
