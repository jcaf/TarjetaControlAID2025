/*
 * pinGetLevel.h
 *
 *  Created on: Dec 3, 2020
 *      Author: jcaf
 */

#ifndef PINGETLEVEL_PINGETLEVEL_H_
#define PINGETLEVEL_PINGETLEVEL_H_

#define PINGETLEVEL_NUMMAX 2 //# of pines to check

/*
struct _pinGetLevel
{
  	int8_t counterDebounce;
    int8_t sm0;
    int8_t level;
    int8_t level_last;
    //
    PTRFX_retUINT8_T readPinLevel;
};
*/
struct _pinGetLevel
{
  	int8_t counterDebounce;
    struct _pinGetLevel_bf
    {
		unsigned sm0:1;
		unsigned level:1;
		unsigned level_last:1;
		unsigned changed:1;
		unsigned __a:4;
  	}bf;

    //
    PTRFX_retUINT8_T readPinLevel;
};

extern struct _pinGetLevel pinGetLevel[PINGETLEVEL_NUMMAX];

//
#define PINGETLEVEL_INITwCHANGED
void pinGetLevel_init();//by default always changed-flag = 1 at begin
//
void pinGetLevel_job(void);

#define pinGetLevel_hasChanged(i)pinGetLevel[i].bf.changed
#define pinGetLevel_setChange(i) do{pinGetLevel[i].bf.changed = 1;}while(0)
#define pinGetLevel_clearChange(i) do{pinGetLevel[i].bf.changed = 0;}while(0)
#define pinGetLevel_level(i) pinGetLevel[i].bf.level


//REMAPING DEFINITIOS PORTW/R
//P1
#define PORTWxINPUT1 PORTD
#define PORTRxINPUT1 PIND
#define CONFIGIOxINPUT1 DDRD
#define PINxINPUT1 4
//P2
#define PORTWxINPUT2 PORTC
#define PORTRxINPUT2 PINC
#define CONFIGIOxINPUT2 DDRC
#define PINxINPUT2 5

//remapping Switch 0
#define PORTWxGETLEVEL_0 	PORTWxINPUT1
#define PORTRxGETLEVEL_0 	PORTRxINPUT1
#define CONFIGIOxGETLEVEL_0 	CONFIGIOxINPUT1
#define PINxGETLEVEL_0		PINxINPUT1

//remapping Switch 0
#define PORTWxGETLEVEL_1 	PORTWxINPUT2
#define PORTRxGETLEVEL_1 	PORTRxINPUT2
#define CONFIGIOxGETLEVEL_1 	CONFIGIOxINPUT2
#define PINxGETLEVEL_1		PINxINPUT2

#define PGLEVEL_LYOUT_KEY_P1UP 0
#define PGLEVEL_LYOUT_KEY_P2DOWN 1



////ONLY FOR TEST
//#include "../ikb/ikb.h"
//#define PORTWxGETLEVEL_1 	PORTWxKB_KEY2
//#define PORTRxGETLEVEL_1 	PORTRxKB_KEY2
//#define CONFIGIOxGETLEVEL_1 	CONFIGIOxKB_KEY2
//#define PINxGETLEVEL_1		PINxKB_KEY2
////
//#define PGLEVEL_LYOUT_CHISPERO 1

//pinGetLevel layout
//#define PGLEVEL_LYOUT_X 1
//#define PGLEVEL_LYOUT_Y 2
//#define PGLEVEL_LYOUT_Z 3


#endif /* PINGETLEVEL_PINGETLEVEL_H_ */
