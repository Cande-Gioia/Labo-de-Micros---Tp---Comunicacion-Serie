
#ifndef _CARDREADERDRV_H_
#define _CARDREADERDRV_H_

#include <stdint.h>
#include <stdbool.h>

#define START_SENTINEL 0xB
#define END_SENTINEL 0xF
#define FEILD_SEPARATOR 0xD

#define NUM_CHARACTERS 200

enum {NO_CARD, CARD_REJECTED, CARD_ACCEPTED};

/*
 * Inicializacion
 *
 */
void cardReaderInit(void);
/*
 * Interrupcion dedicada
 *
 */
void cardReaderISR(void);

/*
 * Interrupcion periodica
 *
 */
void cardReaderPISR(void);

/*
 * Interrupcion periodica para chequear el sentinel
 *
 */
void checkSentinelPISR(void);


uint8_t getBuffer(uint32_t* buff);

#endif
