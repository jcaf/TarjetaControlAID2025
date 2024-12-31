

/*
 * la busqueda en el buffer circular es cada "x" ms
 * no puede ser directa porque perderia mucho tiempo hasta que se complete la trama completa
 *
 * octave:7>  dec2hex(sum(int8('@N512F1023R256')))
	ans = 321 -> el resultado esta en HEX, solo me quedo con el byte menor = 0x21
 *
 * 	@N512F1023R256C21
	@N512F1023R257C22
 */

#include "rx_trama.h"
#include "../serial/serial.h"
#include "../system.h"
#include "../main.h"
#include "../indicator/indicator.h"

#define RX_CSTR_SIZE 32
struct _job_rx
{
	int8_t sm0;
}rx;
void rx_trama(void)
{
	char sbuff_out_temp[SCIRBUF_BUFF_SIZE];

	//Cstr es siempre static porque recolecta y junta todos los caracteres disponibles del buffer serial

	static char Cstr[RX_CSTR_SIZE];//todos los bytes se inicializan a 0
	uint8_t bytes_available;
	char USB_DATACODE = ' ';
	char USB_payload_char[30];
	int8_t USB_payload_idx = 0;
	char c;
	int8_t newData = 0;
	int length;

	float payload_f=0;

	//busqueda en buffer circular
	bytes_available = scirbuf_bytes_available();
	if (bytes_available > 0)
	{
		scirbuf_read_nbytes((uint8_t*)sbuff_out_temp, bytes_available); //hago la copia desde el buffer circular hacia el de salida temporal
		//
		sbuff_out_temp[bytes_available] = '\0';//convertir en c_str
		strcat(Cstr,sbuff_out_temp);

		//
		length = strlen(Cstr);

//usart_print_string(Cstr);

		rx.sm0 = 0;
		for (int i=0; i< length; i++)
		{
			c =  Cstr[i];
			if (rx.sm0 == 0)
			{
				if ( c == USB_DATACODE_TOKEN_BEGIN)
				{
					USB_payload_idx = 0;
					rx.sm0++;
				}
			}
			else if (rx.sm0 == 1)
			{
				USB_DATACODE = c;
				rx.sm0++;
			}
			else if (rx.sm0 == 2)//storage payload
			{
				if (c == USB_DATACODE_TOKEN_END)
				{
					USB_payload_char[USB_payload_idx] = '\0';
					//
					strcpy(Cstr,"");

					rx.sm0 = 0;
					newData = 1;
					break;
				}
				else
				{
					USB_payload_char[USB_payload_idx++] = c;
				}
			}
		}
		if (newData == 1)
		{
			if (USB_payload_idx > 0)	//Si existe payload, tendra payload util
				{payload_f = atof(USB_payload_char);}

			newData = 0;

			//
			indicatorTimed_setKSysTickTime_ms(75/SYSTICK_MS);
			indicatorTimed_run();
			//
			switch (USB_DATACODE)
			{
				//setters
				case USB_DATACODE_RECORRIDO_TOTAL:
					recorrido_total = payload_f;
				break;

				case USB_DATACODE_INTERVALO:
					intervalo = payload_f;
				 break;

				case USB_DATACODE_SET_SELECTOR:
					selector = payload_f;
					set_selector(selector);
					set_ledselector(selector);
				 break;

				case USB_DATACODE_SET_EXECUTION:
					execution = payload_f;
					set_execution(execution);
				 break;

				case USB_DATACODE_SET_MOTOR:
					motor = payload_f;
					set_motor(motor);
				 break;

				case USB_DATACODE_SET_LED_ENLACE:
					led_enlace = payload_f;
					set_led_enlace(led_enlace);
				break;

				//getters, prepare
				case USB_DATACODE_GET_SELECTOR:
					mainflag.usb_send_selector =1;
				break;
				case USB_DATACODE_GET_EXECUTION:
					mainflag.usb_send_execution = 1;
				break;
				case USB_DATACODE_GET_MOTOR:
					mainflag.usb_send_motor = 1;
				break;
				case USB_DATACODE_GET_LED_ENLACE:
					mainflag.usb_send_led_enlace = 1;
				break;

				default:
					break;
			}
		}

	}


}
