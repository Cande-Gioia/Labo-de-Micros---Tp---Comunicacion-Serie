
#ifndef _TIMER_H_
#define _TIMER_H_

enum timers_IDs {TID0,TID1,TID2,TID3,
				TID4,TID5,TID6,TID7,TID8,TID9};

#define MAX_TIMERS 10



void timerInit(uint8_t resolution);

bool timePassed(uint8_t id);

uint32_t getTime(uint8_t id);

void setTimeAndInit(uint8_t id, uint32_t time);



#endif
