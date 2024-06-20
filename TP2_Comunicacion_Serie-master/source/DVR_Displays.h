/*
 * DVR_leds.h
 *
 *  Created on: 28 ago. 2023
 *      Author: grupo 2
 */

#ifndef DVR_DISPLAYS_H_
#define DVR_DISPLAYS_H_


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define CANT_LEDS 8
#define CANT_DISP 4

//vista    	 |1| |1| |1| |1|
enum 		{D0, D1, D2, D3};
enum  {APAGADO=10,LETRA_B,LETRA_N,SIM_TICK};
enum  {DOT_ON, DOT_OFF};

#define LED_ON ACTIVE_LED
#define LED_OFF !ACTIVE_LED
#define MAX_INTENS 5
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef uint32_t delay_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
void ledsInit();
void blinkDisp(uint8_t digito);
void stopBlink(void);
void writeDisp(uint8_t numero, uint8_t digito);
void writeWord(uint8_t * data);
void writeNum(uint64_t num);
void writeDot(uint8_t dot, uint8_t digito);
void time_refresh(uint8_t value);
/*******************************************************************************
 ******************************************************************************/

#endif /* DVR_DISPLAYS_H_ */
