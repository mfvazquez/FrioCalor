#ifndef PRUEBA_H
#define PRUEBA_H

/* ******************************************************************* *
 * 						 			USART                              *
 * ******************************************************************* */

void USART_Init(void);

void transmit(int8_t dato);

int8_t receive(void);

ISR(USART_RXC_vect);

/* ******************************************************************* *
 * 						 			ADC	                               *
 * ******************************************************************* */

void ADC_Init(void);

int8_t obtener_Tamb(void);

int8_t obtener_Tpeltier(void);

/* ******************************************************************* *
 * 						 			PWM	                               *
 * ******************************************************************* */

void PWM_Init(void);

void PWM_on(void);

void PWM_off(void);

void PWM_set(uint8_t pwm);

/* ******************************************************************* *
 * 						 PROGRAMA PRINCIPAL                            *
 * ******************************************************************* */

int main(void);

void loop_transmit(void);

void loop_peltier(int8_t Tdeseada);

void regular_grueso(int8_t dif);

void regular_fino(int8_t Tdeseada, int8_t dif);

char obtener_modo(int8_t Tdeseada);

void aplicar_modo(char modo);

char establecer_modo(int8_t Tdeseada);

void modo_calor(void);

void modo_frio(void);

int8_t traducir_temp(uint8_t volt);

#endif
