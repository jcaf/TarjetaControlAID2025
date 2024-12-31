/*
 * buzzer.c
 *
 *  Created on: Aug 30, 2021
 *      Author: jcaf
 */
#include "../main.h"
#include "indicator.h"

volatile struct _indicator indicator;

void indicator_setPortPin(volatile unsigned char *Port8bits, int8_t pin)
{
	indicator.Port8bits = Port8bits;
	indicator.pin = pin;
}
#ifdef TIMEON_TIMEOFF_SEPARADOS
void indicatorTimed_setKSysTickTimeOn_ms(uint16_t KSysTickTimeOn_ms)///SYSTICK_MS
{
	indicator.KOn_MAX = KSysTickTimeOn_ms;
}
void indicatorTimed_setKSysTickTimeOff_ms(uint16_t KSysTickTimeOff_ms)///SYSTICK_MS
{
	indicator.KOff_MAX = KSysTickTimeOff_ms;
}
#else
void indicatorTimed_setKSysTickTime_ms(uint16_t KSysTickTime_ms)// div by SYSTICK_MS
{
	indicator.KOn_MAX = KSysTickTime_ms;
}
#endif

void indicatorTimed_run(void)//indicator
{
	indicator.sm0 = 1;
}
void indicatorTimed_stop(void)
{
	PinTo0(*indicator.Port8bits, indicator.pin);
	indicator.sm0 = 0;
}
//
void indicatorTimed_cycle_start(void)
{
	indicator.sm0 = 3;
}
/*
 * pero este job es por tiempo, deberia llamarse indicator_timing
 */

void indicator_on(void)
{
	PinTo1(*indicator.Port8bits, indicator.pin);
}
void indicator_off(void)
{
	PinTo0(*indicator.Port8bits, indicator.pin);
}
void indicatorTimed_job(void)
{
	//1 ON, run and stop automatically
	if (indicator.sm0 == 1)
	{
		PinTo1(*indicator.Port8bits, indicator.pin);
		indicator.counter0 = 0;
		indicator.sm0++;
	}
	else if (indicator.sm0 == 2)
	{
		if (mainflag.sysTickMs)
		{
			indicator.counter0++;
			if (indicator.counter0 >= indicator.KOn_MAX)
			{
				indicator.counter0 = 0;
				indicatorTimed_stop();
			}
		}
	}


	//Cycle, stop by user
	if (indicator.sm0 == 3)
	{
		PinToggle(*indicator.Port8bits, indicator.pin);
		indicator.counter0 = 0;
		indicator.sm0++;
	}
	else if (indicator.sm0 == 4)
	{
		if (mainflag.sysTickMs)
		{
			if (++indicator.counter0 >= indicator.KOn_MAX)
			{
				indicator.sm0--;
			}
		}
	}

}
