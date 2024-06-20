/***************************************************************************//**
  @file     can.h
  @brief    Driver CAN Protocol
  @author   Grupo 2
 ******************************************************************************/
#ifndef _CAN_H_
#define _CAN_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
 #include "hardware.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Init CAN driver
 */
void canInit(void);
/**
 * @brief Transmit a message by CAN
 * @param id Can id
 * @param data data to transmit
 * @param lenght amount of bytes to transmit
 * @param id_index MB index
 */
void canTransmit(uint16_t id, uint8_t data[8], uint8_t length, uint8_t id_index);
/**
 * @brief Initialize the MB to be abloe to recieve 
 * @param id Can id
 * @param id_index MB index
 */

void canBuffer_Init(uint16_t id, uint8_t id_index);

/**
 * @brief Recieve data from CAN  
 * @param data the recieve data will be saved here
 * @param id_index MB index
 * @return uint8_t amount of bytes read
 */
uint8_t canRecieve( uint8_t* data,  uint8_t id_index);
#endif // _CAN_H_
