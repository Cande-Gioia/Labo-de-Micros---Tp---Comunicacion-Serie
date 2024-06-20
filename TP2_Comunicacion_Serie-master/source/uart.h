/***************************************************************************//**
  @file     UART.c
  @brief    UART Driver for K64F. Non-Blocking and using FIFO feature
  @author   Nicol�s Magliola
 ******************************************************************************/

#ifndef _UART_H_
#define _UART_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define UART_CANT_IDS  6

enum IDs {UART_0, UART_1, UART_2, UART_3, UART_4, UART_5};

#define TX_BUFFER_SIZE 100;

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
    uint32_t baudrate;
	bool parity;
} uart_cfg_t;


//B , E, C D es 3
// A es 2

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize UART driver
 * @param id UART's number
 * @param config UART's configuration (baudrate, parity, etc.)
*/
void uartInit (uint8_t id, uart_cfg_t config);

/**
 * @brief Devueleve el byte con los flags de status de un uart
 * @param id UART's number
 * @return Uart status byte
*/
uint8_t uartGetStatusFlags(uint8_t id);

//-------------------------------------------receptor
/**
 * @brief Check if a new byte was received
 * @param id UART's number
 * @return A new byte has been received
*/
uint8_t uartIsRxMsg(uint8_t id);

/**
 * @brief Check how many bytes were received
 * @param id UART's number
 * @return Quantity of received bytes
*/
uint8_t uartGetRxMsgLength(uint8_t id);

/**
 * @brief Read a received message. Non-Blocking
 * @param id UART's number
 * @param msg Buffer to paste the received bytes
 * @param cant Desired quantity of bytes to be pasted
 * @return Real quantity of pasted bytes
*/
uint8_t uartReadMsg(uint8_t id, char* msg, uint8_t cant);

//--------------------------------------------transmisor
/**
 * @brief Check if all bytes were transfered
 * @param id UART's number
 * @return All bytes were transfered
*/
uint8_t uartIsTxMsgComplete(uint8_t id);

/**
 * @brief Cuantos bytes se pueden mandar
 * @param id UART's number
 * @return Quantity of transfered bytes
*/
uint8_t uartMaxTxMsgLength(uint8_t id);


/**
 * @brief Write a message to be transmitted. Non-Blocking
 * @param id UART's number
 * @param msg Buffer with the bytes to be transfered
 * @param cant Desired quantity of bytes to be transfered
 * @return Real quantity of bytes to be transfered
*/
uint8_t uartWriteMsg(uint8_t id, unsigned const char* msg, uint8_t cant);


void uartWriteMsgBloq(uint8_t id, const char* msg, uint8_t cant);




/*******************************************************************************
 ******************************************************************************/

#endif // _UART_H_
