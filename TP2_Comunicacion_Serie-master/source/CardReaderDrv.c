#include "CardReaderDrv.h"
#include "SysTick.h"
#include "gpio.h"
#include "board.h"

enum {NOT_READING, READING, CHECK_LRC};


static uint8_t internalState = NOT_READING;
static uint32_t buffer[NUM_CHARACTERS];
static uint8_t sentinel = 0;
static uint32_t count = 5;
static uint8_t state = NO_CARD;
static uint32_t speed = 500;
static bool flag = false;


bool checkEndSentinel(void){
	int counter = count;
	return (
		!buffer[counter-5] &&
		!buffer[counter-4] &&
		!buffer[counter-3] &&
		!buffer[counter-2] &&
		!buffer[counter-1]
	);
}

bool parityCheck(void){
	int num = 0;
	int counter = count;
	for (int i=1; i<=5; i++) {
		if (buffer[counter-i])
			num++;
	}
	return num%2!=0;
}

bool checkRLC(void) {
	int counter = 0;
	for (int i=0; i<4; i++) {
		for (int j=0; j<count; j+=5) {
			counter += !buffer[i+j];
		}
		if  (counter%2!=0)
			return true;
		counter = 0;
	}
	return false;
}

#define MSK2(n) (n&(0xFF>>7))

void sentinel2Buffer(void) {
	for (int i=4; i>=0; i--) {
		buffer[i] = MSK2(sentinel>>i);
	}
}


void cardReaderInit(void) {
	gpioMode(DATA, INPUT);
	gpioMode(ENABLE, INPUT);
	gpioMode(CLK_IN, INPUT);
	gpioIRQ(
		CLK_IN,
		GPIO_IRQ_MODE_FALLING_EDGE,
		cardReaderISR
	);
	SysTick_Init();
	SysTick_Add(checkSentinelPISR);
}


void cardReaderISR(void) {
	switch(internalState) {
	case NOT_READING:
		sentinel >>= 1;
		sentinel += gpioRead(DATA) << 4;
		break;
	case CHECK_LRC:
	case READING:
		buffer[count++] = gpioRead(DATA);
		speed = 500;
		break;
	}

	/*
	if (reading) {
		buffer[count++] = gpioRead(DATA);
		speed = 500;
	}
	else {
		sentinel >>= 1;
		sentinel += gpioRead(DATA) << 4;
	}
	*/
}


#define MSK(n) (n&(0xFF>>3))

void checkSentinelPISR(void) {

	switch (internalState) {
	case NOT_READING:
		if((MSK(sentinel) == MSK((uint8_t)(~START_SENTINEL)))) {
			count = 5;
			sentinel2Buffer();
			internalState = READING;
			state = NO_CARD;
			flag = true;
			gpioWrite(PIN_LED_BLUE, HIGH);
			gpioWrite(PIN_LED_RED, 1);
			sentinel = 0;
			speed = 500;
		}
		break;
	case READING:
		if((count%5 == 0) && (count >=10)) {
			if (checkEndSentinel()) {
				gpioWrite(PIN_LED_GREEN, 1);
				gpioWrite(PIN_LED_BLUE, 0);
				internalState = CHECK_LRC;
				//reading = false;
				//state = CARD_ACCEPTED;
			}
			if (parityCheck()) {
				gpioWrite(PIN_LED_RED, 0);
				gpioWrite(PIN_LED_GREEN, 1);
				internalState = NOT_READING;
				state = CARD_REJECTED;
			}

		}
		speed--;
		if(!speed) {
			internalState = NOT_READING;
			gpioWrite(PIN_LED_GREEN, 1);
			gpioWrite(PIN_LED_RED, 0);
			state = CARD_REJECTED;
		}
		break;
	case CHECK_LRC:
		if(count%5 == 0) {
			if (checkRLC() || parityCheck()) {
				state = CARD_REJECTED;
				internalState = NOT_READING;
				gpioWrite(PIN_LED_RED, 0);
				gpioWrite(PIN_LED_BLUE, HIGH);
			}
			else {
				state = CARD_ACCEPTED;
				internalState = NOT_READING;
				//gpioWrite(PIN_LED_GREEN, 1);
			}
		}
		break;
	}

	if (flag) {
		flag = false;
		gpioWrite(PIN_LED_GREEN, 0);
	}


	/*
	if(!reading) {
		if((MSK(sentinel) == MSK((uint8_t)(~START_SENTINEL)))) {
			reading = true;
			count = 0;
			flag = true;
			gpioWrite(PIN_LED_BLUE, HIGH);
			gpioWrite(PIN_LED_RED, 1);
			sentinel = 0;
			speed = 500;
		}

	}



	if(reading) {
		if((count%5 == 0) && (count >=5)) {
			if (checkEndSentinel()) {
				gpioWrite(PIN_LED_GREEN, 1);
				gpioWrite(PIN_LED_BLUE, 0);
				internalState = CHECK_LRC;
				//reading = false;
				//state = CARD_ACCEPTED;
			}
			if (parityCheck()) {
				gpioWrite(PIN_LED_RED, 0);
				gpioWrite(PIN_LED_GREEN, 1);
				reading = false;
				state = CARD_REJECTED;
			}

		}
		speed--;
		if(!speed) {
			reading = false;
			gpioWrite(PIN_LED_GREEN, 1);
			gpioWrite(PIN_LED_RED, 0);
			state = CARD_REJECTED;
		}
	}
	*/
}

uint8_t getBuffer(uint32_t* buff) {
	int oldState = state;
	state = NO_CARD;
	if (oldState == CARD_ACCEPTED) {
		for(int i = 0; i < NUM_CHARACTERS; i++)
			if (i < count)
				buff[i] = buffer[i];
			else
				buff[i] = 1;
		int a = 0;
		a++;
	}
	return oldState;
}


