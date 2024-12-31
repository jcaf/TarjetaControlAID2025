/*
 * main.c
 *
 *  Created on: 20 dic. 2024
 *      Author: JCaf
 *      Atmega328P @20MHz
 *
 *Comprobar USART online
 *https://wormfood.net/avrbaudcalc.php?bitrate=19.2k&clock=20&databits=8
 *
 *Terminal serial port at 230400 bauds:
 *https://sourceforge.net/projects/realterm/
 *
 *Encoder
 *http://makeatronics.blogspot.com/2013/02/efficiently-reading-quadrature-with.html

CH340 DATASHEET
https://www.mpja.com/download/35227cpdata.pdf
 * ----------------------------------------
 1)  http://www.engbedded.com/fusecalc/
 	 lock bits:
 	 http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p

 2) verificar que responda el atmega (ONLY A RESET)
 [jcaf@jcafpc ~]$ avrdude -c usbasp -B5 -v -p m328P

 3) programar fuse (sin preservar EEPROM)

 [jcaf@jcafpc ~]$ avrdude -c usbasp -B5 -p m328P -U lfuse:w:0xff:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m

 4) GRABAR EL CODIGO FUENTE CON EL COMANDO ACOSTUMBRADO
 [root@JCAFPC Release]# avrdude -c usbasp -B5 -p m328P -U flash:w:TarjetaControlAID2025.hex

 NUEVO
 [root@JCAFPC Release]# avrdude -c usbasp -B0.3 -p m328P -V -U flash:w:Temporizador4.hex (MAS RAPIDO!)
 Tambien puede ser sin -BX.. cuando ya esta bien configurado los fuses:
 [root@JCAFPC Release]# avrdude -c usbasp -p m328P -U flash:w:Temporizador4.hex

 5) GRABAR LA EEPROM
 [jcaf@jcafpc Release]$ avrdude -c usbasp -B4 -p m328P -V -U eeprom:w:Temporizador4.eep

 6) programar fuse (PRESERVANDO EEPROM)

		avrdude -c usbasp -B5 -p m328P -U lfuse:w:0xff:m -U hfuse:w:0xd1:m -U efuse:w:0xff:m

 7) Verificar los fuses
 [jcaf@jcafpc Release]$ avrdude -c usbasp -B4 -p m328P -U lfuse:r:-:i -v

 +++++++++++++++++++++++
 proteger flash (modo 3): lectura y escritura
 avrdude -c usbasp -B10 -p m328P -U lock:w:0xFC:m

BitRate UART
19.2k @ 20MHz

https://wormfood.net/avrbaudcalc.php?postbitrate=9600&postclock=1
 */

#include "system.h"
#include "main.h"
#include "rx_trama/rx_trama.h"
#include "pinGetLevel/pinGetLevel.h"
#include "indicator/indicator.h"
#include "usart/usart.h"
#include "serial/serial.h"


volatile  int32_t enc_count = 0;
//

float recorrido_total;
float intervalo;
int selector;
int execution;
int motor;
int led_enlace;

float recorrido;

volatile struct _isr_flag
{
	unsigned sysTickMs :1;
	unsigned sendRecorrido :1;
	unsigned __a :6;
} isr_flag = { 0 };

struct _mainflag mainflag;

volatile static uint8_t enc_val = 0;

//+- Encoder
//void encoder_reset(void);
////typedef int64_t ROTARYCOUNT_T;
//typedef int32_t ROTARYCOUNT_T;
//struct _encoder
//{
//	struct _encoder_flag
//	{
//		unsigned commingFromInc:1;
//		unsigned commingFromDec:1;
//		unsigned update:1;
//		unsigned __a:5;
//	}f;
//	int8_t count4edges;;
//	ROTARYCOUNT_T rotaryCount;
//};
//volatile struct _encoder encoder;
//const struct _encoder encoderReset;

//uint16_t ENCODER_PPR = 500;    			//500 Pulses Per Revolution
//float ENCODER_1REV_INMETERS = 0.5f;    	//1revol = X meters
//volatile float ADQ_KMETERS = 1.0f;		//Adquirir cada "x metros"
//float ENC_RESOL = 0;// = (float)ENCODER_1REV_INMETERS/ENCODER_PPR;
////
////Las sgtes. variables no necesitan ser de 64bits
//int32_t numPulsesIn_ADQ_KMETERS = 0; //(ADQ_KMETERS * ENCODER_PPR) / ENCODER_1REV_INMETERS;//truncar
//int32_t numPulses_diff = 0;
//
volatile uint8_t old_PORTRxENC_CHB;//track last change in quadrature
//

void set_execution(int execution)
{
	if (execution == INICIO)
	{
		set_motor(MOTOR_ON);
		set_led_motor(MOTOR_ON);
		//activar control
	}
	else if (execution == PAUSA)
	{
		set_motor(MOTOR_OFF);//espera a la orden de inicio
		set_led_motor(MOTOR_OFF);
	}
	else if (execution == PARAR)
	{
		set_motor(MOTOR_OFF);//espera a la orden de inicio
		set_led_motor(MOTOR_OFF);
	}
	else if (execution == RESET)
	{
		set_motor(MOTOR_OFF);//espera a la orden de inicio
		set_led_motor(MOTOR_OFF);
		//
		recorrido = 0;
	}
}

void set_led_enlace(int led_enlace)
{
	if (led_enlace == 1)
	{
		PinTo1(PORTWxLED_ENLACE,PINxLED_ENLACE);
	}
	else
	{
		PinTo0(PORTWxLED_ENLACE,PINxLED_ENLACE);
	}
}

void set_led_motor(int led_motor)
{
	if (led_motor == 1)
	{
		PinTo1(PORTWxLED_MOTOR_ACTIVADO,PINxLED_MOTOR_ACTIVADO);
	}
	else
	{
		PinTo0(PORTWxLED_MOTOR_ACTIVADO,PINxLED_MOTOR_ACTIVADO);
	}
}
void set_motor(int motor)
{
	if (motor == 1)
	{
		PinTo1(PORTWxRELAY_MOTOR_GIRO,PINxRELAY_MOTOR_GIRO);
	}
	else
	{
		PinTo0(PORTWxRELAY_MOTOR_GIRO,PINxRELAY_MOTOR_GIRO);
	}
}
void set_ledselector(int selector)
{
	if (selector == SELECTOR_SP)
	{
		PinTo1(PORTWxLED1,PINxLED1);
		PinTo0(PORTWxLED2,PINxLED2);
		PinTo0(PORTWxLED3,PINxLED3);
		PinTo0(PORTWxLED4,PINxLED4);
	}
	else if (selector == SELECTOR_NC)
	{
		PinTo0(PORTWxLED1,PINxLED1);
		PinTo1(PORTWxLED2,PINxLED2);
		PinTo0(PORTWxLED3,PINxLED3);
		PinTo0(PORTWxLED4,PINxLED4);
	}
	else if (selector == SELECTOR_NL)
	{
		PinTo0(PORTWxLED1,PINxLED1);
		PinTo0(PORTWxLED2,PINxLED2);
		PinTo1(PORTWxLED3,PINxLED3);
		PinTo0(PORTWxLED4,PINxLED4);
	}
	else if (selector == SELECTOR_L)
	{
		PinTo0(PORTWxLED1,PINxLED1);
		PinTo0(PORTWxLED2,PINxLED2);
		PinTo0(PORTWxLED3,PINxLED3);
		PinTo1(PORTWxLED4,PINxLED4);
	}

}
void set_selector(int selector)
{
	if (selector == SELECTOR_SP)
	{
		//RLY2
		PinTo0(PORTWxRELAY1,PINxRELAY1);
		PinTo1(PORTWxRELAY2,PINxRELAY2);
		PinTo0(PORTWxRELAY3,PINxRELAY3);
		PinTo0(PORTWxRELAY4,PINxRELAY4);
	}
	else if (selector == SELECTOR_NC)
	{
		//RLY1 - RLY2
		PinTo1(PORTWxRELAY1,PINxRELAY1);
		PinTo1(PORTWxRELAY2,PINxRELAY2);
		PinTo0(PORTWxRELAY3,PINxRELAY3);
		PinTo0(PORTWxRELAY4,PINxRELAY4);
	}
	else if (selector == SELECTOR_NL)
	{
		//RLY1 - RLY3
		PinTo1(PORTWxRELAY1,PINxRELAY1);
		PinTo0(PORTWxRELAY2,PINxRELAY2);
		PinTo1(PORTWxRELAY3,PINxRELAY3);
		PinTo0(PORTWxRELAY4,PINxRELAY4);
	}
	else if (selector == SELECTOR_L)
	{
		//RLY1 - RLY4
		PinTo1(PORTWxRELAY1,PINxRELAY1);
		PinTo0(PORTWxRELAY2,PINxRELAY2);
		PinTo0(PORTWxRELAY3,PINxRELAY3);
		PinTo1(PORTWxRELAY4,PINxRELAY4);
	}
}

/*
void USB_send_data_float(char datacode, float payload0)
{
	char str[30];
	char buff[30];

	str[0] = USB_DATACODE_TOKEN_BEGIN;
	str[1] = datacode;
	str[2] = '\0';


//	if (datacode == USB_DATACODE_POSICION)
//	{
//		dtostrf(payload0, 0, 3, buff);//solo 3 decimales
//	}
//	else
//	{
//		dtostrf(payload0, 0, 2, buff);//current in mV + mA
//	}

	strcat(str,buff);
	strcat(str,"\r");

	//usart_println_string(str);
	usart_print_string(str);
}
*/
void USB_send_data_integer(char datacode, int payload0)
{
	char str[30];
	char buff[30];

	str[0] = USB_DATACODE_TOKEN_BEGIN;
	str[1] = datacode;
	str[2] = '\0';
	itoa(payload0, buff, 10);
	strcat(str,buff);
	//strcat(str,"\r");
	strcat(str,"\n");

	//usart_println_string(str);
	usart_print_string(str);
}
int main(void)
{
	int counter0=0;
	char buff[10];

	//Activar pullups en las entradas
	pinGetLevel_init(); //with Changed=flag activated at initialization

	ConfigOutputPin(CONFIGIOxRELAY1, PINxRELAY1);
	ConfigOutputPin(CONFIGIOxRELAY2, PINxRELAY2);
	ConfigOutputPin(CONFIGIOxRELAY3, PINxRELAY3);
	ConfigOutputPin(CONFIGIOxRELAY4, PINxRELAY4);

	ConfigOutputPin(CONFIGIOxLED_MOTOR_ACTIVADO, PINxLED_MOTOR_ACTIVADO);
	ConfigOutputPin(CONFIGIOxLED_ENLACE, PINxLED_ENLACE);
	ConfigOutputPin(CONFIGIOxLED1, PINxLED1);
	ConfigOutputPin(CONFIGIOxLED2, PINxLED2);
	ConfigOutputPin(CONFIGIOxLED3, PINxLED3);
	ConfigOutputPin(CONFIGIOxLED4, PINxLED4);

	ConfigOutputPin(CONFIGIOxRELAY_MOTOR_GIRO, PINxRELAY_MOTOR_GIRO);

	ConfigOutputPin(CONFIGIOxBUZZER, PINxBUZZER);
	indicator_setPortPin(&PORTWxBUZZER, PINxBUZZER);
	//indicatorTimed_setKSysTickTime_ms(75/SYSTICK_MS);
	indicatorTimed_setKSysTickTime_ms(1000/SYSTICK_MS);
	indicatorTimed_run();


	ConfigInputPin(CONFIGIOxENC_CHA, PINxENC_CHA);
	ConfigInputPin(CONFIGIOxENC_CHB, PINxENC_CHB);



	//With prescaler 64, gets 1 ms exact (OCR0=249) @16mhz
	//With prescaler 256, gets 1 ms no-exact (OCR0=77) @20mhz
	//Config to 1ms
	TCNT0 = 0x00;
	TCCR0A = (1 << WGM01);
	TCCR0B =  (1 << CS02) | (0 << CS01) | (0 << CS00); //CTC, PRES=256
	OCR0A = CTC_SET_OCR_BYTIME(1e-3, 256);// 77.125
	//
	TIMSK0 |= (1 << OCIE0A);
	//sei();

	//USART_Init ( 64);//38400 baudios
//	USART_Init ( (int)MYUBRR);
	USART_Init ( 10);

//	while (1)
//	{	usart_print_string("38400");
//		__delay_ms(1000);
//	}
	//Encoder setup Atmega328P, external Pull-ups 1K
	//channel A = PD2 INT0 / PCINT18
	//channel B = PD3 INT1 / PCINT19
	PCICR 	= 0x04;//PCIE2 PCINT[23:16] Any change on any enabled PCINT[23:16] pin will cause an interrupt.
	PCMSK2 	= 0x0C;//PCINT18 PCINT19
	//old_PORTRxENC_CHB = PORTRxENC_CHB;

	enc_val = ((PIND & 0b00001100) >> 2);
	sei();

	while (1)
	{
		if (isr_flag.sysTickMs)
		{
			isr_flag.sysTickMs = 0;
			mainflag.sysTickMs = 1;
		}

		if (mainflag.sysTickMs)
		{
			if (++counter0 == (20/SYSTICK_MS))    //20ms
			{
				counter0 = 0;
				//
ltoa(enc_count, buff, 10);
usart_println_string(buff);


				pinGetLevel_job();
				//UP
				if (pinGetLevel_hasChanged(PGLEVEL_LYOUT_KEY_P1UP))
				{
					pinGetLevel_clearChange(PGLEVEL_LYOUT_KEY_P1UP);
					//
					if (pinGetLevel_level(PGLEVEL_LYOUT_KEY_P1UP)== 0)	//active in low
					{
						if (++selector > SELECTOR_L)
						{
							selector = SELECTOR_SP;
						}
						set_selector(selector);
						set_ledselector(selector);
						//
						USB_send_data_integer(USB_DATACODE_SET_SELECTOR, selector);

						//
						indicatorTimed_setKSysTickTime_ms(75/SYSTICK_MS);
						indicatorTimed_run();

					}

				}
				//DOWN
				if (pinGetLevel_hasChanged(PGLEVEL_LYOUT_KEY_P2DOWN))
				{
					pinGetLevel_clearChange(PGLEVEL_LYOUT_KEY_P2DOWN);
					//
					if (pinGetLevel_level(PGLEVEL_LYOUT_KEY_P2DOWN)== 0)	//active in low
					{
						if (--selector < SELECTOR_SP)
						{
							selector = SELECTOR_L;
						}
						set_selector(selector);
						set_ledselector(selector);
						//
						USB_send_data_integer(USB_DATACODE_SET_SELECTOR, selector);

						//
						indicatorTimed_setKSysTickTime_ms(75/SYSTICK_MS);
						indicatorTimed_run();

					}
				}

			}
		}

		//
		if (mainflag.control_recorrido == 1)
		{
			if (recorrido >= intervalo)
			{
				set_motor(MOTOR_OFF);//espera a la orden de inicio
				set_led_motor(MOTOR_OFF);
			}
		}
		//Send data to Host PC
		if (mainflag.usb_send_selector)
		{
			USB_send_data_integer(USB_DATACODE_SET_SELECTOR, selector);
			mainflag.usb_send_selector =0;
		}
		if (mainflag.usb_send_execution)
		{
			USB_send_data_integer(USB_DATACODE_SET_EXECUTION, execution);
			mainflag.usb_send_execution = 0;
		}
		if (mainflag.usb_send_motor)
		{
			USB_send_data_integer(USB_DATACODE_SET_MOTOR, motor);
			mainflag.usb_send_motor = 0;
		}
		if (mainflag.usb_send_led_enlace)
		{
			USB_send_data_integer(USB_DATACODE_SET_LED_ENLACE, led_enlace);
			mainflag.usb_send_led_enlace = 0;
		}

		if (isr_flag.sendRecorrido)
		{
//			itoa(enc_count, buff, 10);
//			usart_println_string(buff);
			isr_flag.sendRecorrido = 0;
		}

		//++
		indicatorTimed_job();
		mainflag.sysTickMs = 0;
	}
	return 0;
}

ISR(TIMER0_COMPA_vect)
{
	isr_flag.sysTickMs = 1;
}

//void encoder_reset(void)
//{
//	//
//	PCICR 	= 0x00;//disable PCIE2 PCINT[23:16]
//	//encoder.rotaryCount = 0x0000;
//	encoder = encoderReset;//clear struct
//	old_PORTRxENC_CHB = PORTRxENC_CHB;
////	isr_flag.send_posicion = 1;
//
//	PCIFR 	= 0x04;//PCINT18 PCINT19 clear flags
//	PCICR 	= 0x04;//PCIE2 PCINT[23:16] Any change on any enabled PCINT[23:16] pin will cause an interrupt.
//
//}
//void encoder_read(void)
//{
//	enc_val = enc_val << 2;
//	enc_val = enc_val | ((PIND & 0b00001100) >> 2);
//
//	enc_count = enc_count + lookup_table[enc_val & 0b00001111];
//}

//Este ISR es por cambio de nivel
ISR(PCINT2_vect)
{
//	static int8_t lookup_table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
//	static uint8_t enc_val = 0;
	//
	//CANAL A DEBE DE ESTAR EN PD3 Y CANAL B EN PD2 para esta secuencia:
	//volatile static int8_t lookup_table[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
	//pero como esta al reves, he invertido la tabla lookup
	volatile static int8_t lookup_table[] = {0,1,-1,0,-1,0,0,1,1,0,0,-1,0,-1,1,0};
	//volatile static uint8_t enc_val = 0;

	enc_val = enc_val << 2;
	enc_val = enc_val | ((PIND & 0b00001100) >> 2);

	enc_count = enc_count + lookup_table[enc_val & 0b00001111];
	isr_flag.sendRecorrido = 1;
}
/*
ISR(PCINT2_vect)//void encoder_xor(void)
{
	uint8_t direction;
	//volatile static int8_t count4edges = 0;
	//
	direction = (PORTRxENC_CHA ^ (old_PORTRxENC_CHB>>1) ) & (1<<PINxENC_CHA);
	old_PORTRxENC_CHB = PORTRxENC_CHB;//save CHB



	if (direction != 0 )
	{
		encoder.f.commingFromInc = 1;
		if (encoder.count4edges>=4)
		{
			encoder.count4edges = 0;
		}
		encoder.count4edges++;
		if (encoder.count4edges == 4)
		{
			if (encoder.f.commingFromDec == 1)
			{
				encoder.f.commingFromDec = 0;
			}
			else
			{
				//encoder.f.update = 1;

				//++-Solo para hacer +rapido el ISR
				encoder.rotaryCount++;
				isr_flag.send_posicion = 1;

				//
				if (encoder.rotaryCount == 0)
				{
					isr_flag.posicion_0 = 1;
				}

				//-++
			}
		}
	}
	else
	{
		encoder.f.commingFromDec = 1;
		if (encoder.count4edges<=0)
		{
			encoder.count4edges = 4;
		}
		encoder.count4edges--;
		if (encoder.count4edges == 0)
		{
			if (encoder.f.commingFromInc == 1)
			{
				encoder.f.commingFromInc = 0;
			}
			else
			{
				//encoder.f.update = 1;

				//++-Solo para hacer +rapido el ISR
				encoder.rotaryCount--;
				isr_flag.send_posicion = 1;
				//-++
				//
				if (encoder.rotaryCount == 0)
				{
					isr_flag.posicion_0 = 1;
				}
			}
		}
	}
}
*/
