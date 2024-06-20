/***************************************************************************//**
  @file     I2C.H
  @brief    I2C Driver for K64F.
  @author   Grupo 2
 ******************************************************************************/

#ifndef _I2C_H_
#define _I2C_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define I2C_CANT_IDS  3

enum {I2C_0, I2C_1, I2C_2};

#define TX_BUFFER_SIZE 50;
#define RX 0
#define TX 1
#define RS 3 //repeat start
#define MAX_DATA 50
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
    uint32_t baudrate;
    uint8_t address;
    uint8_t mode;
    uint8_t cant_bytes_rx;
    uint8_t cant_bytes_tx;
    uint8_t data[MAX_DATA];
} i2c_cfg_t;



/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize I2C driver
 * @param id I2C's number
 * @param config I2C's configuration (baudrate, parity, etc.)
*/
void i2cInit (uint8_t id);


/**
 * @brief Initialize master for I2C driver
 * @param id I2C's number
 * @param config I2C's configuration (baudrate, parity, etc.)
 * @param mul Multiplier Factor
 * @param icr ClockRate
*/
void i2cInit_master (uint8_t id, i2c_cfg_t* config, uint8_t mul, uint8_t icr);

/**
 * @brief Set address slave 
 * @param address address slave
 * @param id I2C's number
 * @param tx_rx Recieve = 0, Transmit = 1
 * 
*/
void set_adress_slave(uint8_t address, uint8_t id, uint8_t tx_rx);

/**
 * @brief Define transmit o recieve
 * @param id I2C's number
 * @param tx_rx Recieve = 0, Transmit = 1
 * 
*/
void set_tx_rx(uint8_t tx_rx, uint8_t id);

/**
 * @brief Get recieve data
 * @param id I2C's number
 * @return uint8_t data recieved
*/
uint8_t get_rx_data(uint8_t id);
/**
 * @brief Get bytes recieved
 * @param id I2C's number
 * @return uint8_t bytes recieved
*/
uint8_t get_rx_count_data(uint8_t id);
/**
 * @brief Check if bus is busy
 * @param id I2C's number
 * @return bool busy = true, not_busy = false
*/
bool I2CisBusy(uint8_t id) ;
/*******************************************************************************
 ******************************************************************************/

#endif // _I2C_H_
