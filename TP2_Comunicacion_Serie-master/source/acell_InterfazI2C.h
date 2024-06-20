/*******************************************************************************
  @file     +acell_InterfazI2C.h+
  @brief    +Driver para I2C del FXOS8700CQ+
  @author   +GRUPO 2+
 ******************************************************************************/
#ifndef ACELL_INTERFAZI2C_H_
#define ACELL_INTERFAZI2C_H_


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
enum slaves_id {SLAVE0,SLAVE1,SLAVE2,SLAVE3};
enum modes {ACELL_ONLY,MAG_ONLY,HYBRID=3};
#define DATA_MSB_2_INT16(x)	 	( ((int16_t)((x)<<8)) >>2 )


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef struct{
	uint8_t reg;
	uint8_t * values;
	uint8_t cant_values;
} message_Type;


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
/*
 * brief: Inicializa comunicacion por I2C y Driver de accel
 * params: 	+slave_id: indica ID del accelerometro
 * 			+mode: valor del enum mode indicando el uso del FOXS8700CQ
 */
int8_t initI2C_FXOS(uint8_t slave_id, uint8_t mode);

/*
 *	brief: realiza el pedido de los datos XYZ
 *	params: +slave_id: indica ID del accelerometro
 * 			+buff: puntero al buff donde se deben guardar los datos
 */
void requestDataXYZ(uint8_t slave_id,  uint16_t * xyz_accel, uint16_t * xyz_mag);

/*
 *	brief: realiza la escritura sobre un registro
 *	params: +slave_id: indica ID del accelerometro
 * 			+message: puntero a estructura con datos a mandar
 */
uint8_t sendMessage(uint8_t slave_id, message_Type * message);

/*
 *	brief: realiza la lectuar sobre un registro
 *	params: +slave_id: indica ID del accelerometro
 * 			+message: puntero a estructura con datos a mandar (para leer)
 */
uint8_t readRegisters(uint8_t slave_id, message_Type * message, uint8_t * regData);

#endif /* ACELL_INTERFAZI2C_H_ */
