#include "SysTick.h"
#include "timer.h"


#define INTERVAL_MICROS 100

typedef struct{
	uint32_t time;
	uint8_t flag_init;
	uint8_t flag_finish;
}timer_Type;


static void timersPISR(void);

static timer_Type timers[MAX_TIMERS];
static uint16_t interval;
static uint8_t nTicksRes;

//resolution en milis
void timerInit(uint8_t resolution) {
	nTicksRes = (resolution * 1000 )/ INTERVAL_MICROS;
	interval = resolution;
	uint8_t i = 0;
	for(i = 0; i < MAX_TIMERS; i++){
		timers[i].flag_finish = 0;
		timers[i].flag_init = 0;
		timers[i].time = 0;
	}

	SysTick_Init();
	SysTick_Add(timersPISR);
}

void setTimeAndInit(uint8_t id, uint32_t time){
	timers[id].time = time;
	timers[id].flag_finish = 0;
	timers[id].flag_init = 1;
}


bool timePassed(uint8_t id) {
	bool status = timers[id].flag_finish;
	if(status){
		timers[id].flag_finish=0;
	}
	return status;
}


uint32_t getTime(uint8_t id) {
	return timers[id].time;//*(TIME_LOAD/SYSTICK_ISR_FREQUENCY_MHZ);
}


static void timersPISR(void) {
	static uint8_t ticks = 1;
	if(!--ticks){
		ticks = nTicksRes;
		uint8_t i = 0;
		for(i = 0; i < MAX_TIMERS; i++){
			if(timers[i].time > interval){
				timers[i].time -= interval;
			}
			else{
				timers[i].flag_finish = 1;
			}
		}
	}
}


