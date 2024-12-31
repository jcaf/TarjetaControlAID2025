/*
 * main.h
 *
 *  Created on: 20 dic. 2024
 *      Author: NEFER
 */

#ifndef MAIN_H_
#define MAIN_H_

#include "system.h"
#include "types.h"


#define SYSTICK_MS 1	//1ms

struct _mainflag
{
		unsigned sysTickMs :1;
		unsigned control_recorrido:1;
		unsigned usb_send_selector:1;
		unsigned usb_send_execution:1;
		unsigned usb_send_motor:1;
		unsigned usb_send_led_enlace:1;
		unsigned __a:2;
};

extern struct _mainflag mainflag;

void set_execution(int execution);
void set_led_enlace(int led_enlace);
void set_led_motor(int led_motor);
void set_motor(int motor);
void set_ledselector(int selector);
void set_selector(int selector);

#define PORTWxRELAY1 PORTD
#define PORTRxRELAY1 PIND
#define CONFIGIOxRELAY1 DDRD
#define PINxRELAY1 6

#define PORTWxRELAY2 PORTD
#define PORTRxRELAY2 PIND
#define CONFIGIOxRELAY2 DDRD
#define PINxRELAY2 7

#define PORTWxRELAY3 PORTB
#define PORTRxRELAY3 PINB
#define CONFIGIOxRELAY3 DDRB
#define PINxRELAY3 0

#define PORTWxRELAY4 PORTB
#define PORTRxRELAY4 PINB
#define CONFIGIOxRELAY4 DDRB
#define PINxRELAY4 1

#define PORTWxLED_MOTOR_ACTIVADO PORTB
#define PORTRxLED_MOTOR_ACTIVADO PINB
#define CONFIGIOxLED_MOTOR_ACTIVADO DDRB
#define PINxLED_MOTOR_ACTIVADO 2

#define PORTWxBUZZER PORTB
#define PORTRxBUZZER PINB
#define CONFIGIOxBUZZER DDRB
#define PINxBUZZER 3

#define PORTWxLED_ENLACE PORTC
#define PORTRxLED_ENLACE PINC
#define CONFIGIOxLED_ENLACE DDRC
#define PINxLED_ENLACE 0


#define PORTWxLED1 PORTC
#define PORTRxLED1 PINC
#define CONFIGIOxLED1 DDRC
#define PINxLED1 1

#define PORTWxLED2 PORTC
#define PORTRxLED2 PINC
#define CONFIGIOxLED2 DDRC
#define PINxLED2 2

#define PORTWxLED3 PORTC
#define PORTRxLED3 PINC
#define CONFIGIOxLED3 DDRC
#define PINxLED3 3

#define PORTWxLED4 PORTC
#define PORTRxLED4 PINC
#define CONFIGIOxLED4 DDRC
#define PINxLED4 4


#define PORTWxRELAY_MOTOR_GIRO PORTD
#define PORTRxRELAY_MOTOR_GIRO PIND
#define CONFIGIOxRELAY_MOTOR_GIRO DDRD
#define PINxRELAY_MOTOR_GIRO 5


//Encoder
#define PORTWxENC_CHA  	PORTD
#define PORTRxENC_CHA		PIND
#define CONFIGIOxENC_CHA 	DDRD
#define PINxENC_CHA 2

#define PORTWxENC_CHB  	PORTD
#define PORTRxENC_CHB		PIND
#define CONFIGIOxENC_CHB 	DDRD
#define PINxENC_CHB 3


//////////////////////////////////////////////////////////
#define USB_DATACODE_TOKEN_BEGIN '@'
#define USB_DATACODE_TOKEN_END '\r'

#define USB_DATACODE_RECORRIDO_TOTAL 'A'
#define USB_DATACODE_INTERVALO 'B'

#define USB_DATACODE_SET_SELECTOR 'C'
	#define SELECTOR_SP 0
	#define SELECTOR_NC 1
	#define SELECTOR_NL 2
	#define SELECTOR_L  3

#define USB_DATACODE_SET_EXECUTION 'D'
	#define INICIO 0
	#define PAUSA 1
	#define PARAR 2
	#define RESET 3

#define USB_DATACODE_SET_MOTOR 'E'
	#define MOTOR_ON 1
	#define MOTOR_OFF 0

#define USB_DATACODE_SET_LED_ENLACE 'F'
	#define LED_ENLACE_ON 1
	#define LED_ENLACE_OFF 0

////////////////////////////////////
#define USB_DATACODE_GET_SELECTOR 'G'
#define USB_DATACODE_GET_EXECUTION 'H'
#define USB_DATACODE_GET_MOTOR 'I'
#define USB_DATACODE_GET_LED_ENLACE 'J'

extern float recorrido_total;
extern float intervalo;
extern int selector;
extern int execution;
extern int motor;
extern float posicion;
extern int led_enlace;

#endif /* MAIN_H_ */
