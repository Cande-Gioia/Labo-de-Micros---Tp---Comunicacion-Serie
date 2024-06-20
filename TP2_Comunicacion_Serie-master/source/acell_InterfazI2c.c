/*******************************************************************************
  @file     +acell_InterfazI2C.c+
  @brief    +interfaz para I2C del FXOS8700CQ+
  @author   +GRUPO 2+
 ******************************************************************************/
/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "acell_InterfazI2C.h"
#include "FXOS8700CQ.h"
#include "i2c.h"
#include "SysTick.h"
#include "cola.h"
#include <stdbool.h>
#include "gpio.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define NUM_BYTES_XYZ 6
#define TICKS_I2C_POLL 2

/*
 * De dos bytes (int8_t) devuelve
 */
#define TWO_I8_2_ONE_I16(b0,b1) ( ((int16_t)(b1))<<8 | (int16_t)(b0) )
#define DATA_MSB14B_2_I16(x)	( (int16_t)(x) << 6)
//<<6

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
enum {X_MSB, X_LSB, Y_MSB, Y_LSB, Z_MSB, Z_LSB};



/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
static void PISR_request_XYZ_Data(void);


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint8_t const slaves_addrs[CANT_SLAVE] = {FXOS8700CQ_SLAVE_ADDR0,
		FXOS8700CQ_SLAVE_ADDR1,FXOS8700CQ_SLAVE_ADDR2,FXOS8700CQ_SLAVE_ADDR3};


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint8_t isBusyI2C;
static uint8_t slaveInI2C;
static uint16_t * slaveXYZDataAccelMSB;
static uint16_t * slaveXYZDataMagMSB;
static uint8_t runPISR;
static uint8_t numCoord;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

int8_t initI2C_FXOS(uint8_t slave_id, uint8_t mode){
	int8_t status = 0;  //status 	- - - -	 d3 d2 d1 d0
	//d0 error send; d1 error CTR1, d2 error M_CTR1,d3 error M_CTR2,

	i2cInit(I2C_0);
	numCoord = (mode==HYBRID) ? 6 : 3;

	//config accel
	message_Type message;
	uint8_t mess[2] = {0};

	//CTRL_REG1
	uint8_t odr = (mode ? ODR_800 : ODRH_400);
		mess[0] = FXOS_CR1_DR(odr) | FXOS_CR1_F_READ_MASK | FXOS_CR1_ACTIVE_MASK;

		message.reg = FXOS8700CQ_CTRL_REG1;
		message.values = mess;
		message.cant_values = 1;
		status |= sendMessage(slave_id,&message);
		while(I2CisBusy(I2C_0));

		uint32_t veces = 800000;
		while(veces--);


		uint8_t reg_2 = 0;
		readRegisters(slave_id, &message, &reg_2);
	//	while(I2CisBusy(I2C_0));
		veces = 800000;
		while(veces--);
		reg_2 = get_rx_data(I2C_0);
		status |= (mess[0]==reg_2 ? 0x00 : 0x02);


		message.cant_values = 2;
		message.reg = FXOS8700CQ_M_CTRL_REG1;
		mess[0] = FXOS_M_CR1_HMS_MASK(mode);
		mess[1] = FXOS_M_CR2_H_AUTOINC_MASK;
		status |= sendMessage(slave_id, &message);
		while(I2CisBusy(I2C_0));
		veces = 800000;
		while(veces--);

		uint8_t reg_m_ctr[2] = {0};
		message.reg = FXOS8700CQ_M_CTRL_REG1;
		message.cant_values = 2;
		readRegisters(slave_id, &message, reg_m_ctr);
		veces = 800000;
		while(veces--);
		uint8_t i = 0;
		for(i = 0; i < message.cant_values; i++){
			reg_m_ctr[i] = get_rx_data(I2C_0);
		}
		status |= (mess[0]==reg_m_ctr[0] ? 0x00 : 0x04);
		status |= (mess[1]==reg_m_ctr[1] ? 0x00 : 0x08);





	SysTick_Init();
	SysTick_Add(PISR_request_XYZ_Data);
	return status;
}


void requestDataXYZ(uint8_t slave_id, uint16_t * xyz_accel, uint16_t * xyz_mag){
	//message to I2C request DR
	if(!isBusyI2C){
		isBusyI2C = 1;
		slaveInI2C = slaves_addrs[slave_id];
		slaveXYZDataAccelMSB = xyz_accel;
		slaveXYZDataMagMSB = xyz_mag;

		i2c_cfg_t master = {.address=slaves_addrs[slave_id], .mode = RS,
						.cant_bytes_tx = 1, .cant_bytes_rx = 1};
		master.data[0] = FXOS8700CQ_DR_STATUS;
		i2cInit_master(I2C_0, &master, 0, 0x30);

		//Enable PISR
		runPISR = 1;
	}
}

uint8_t sendMessage(uint8_t slave_id, message_Type * message){
	if(!isBusyI2C){
		isBusyI2C = 1;
		i2c_cfg_t master = {.address=slaves_addrs[slave_id],.mode=TX,
				.cant_bytes_tx=message->cant_values + 1};
		master.data[0] = message->reg;
		uint8_t i = 0;
		for(i = 0; i< message->cant_values; i++){
			master.data[i+1] = message->values[i];
		}
		i2cInit_master(I2C_0, &master, 0, 0x30);
		isBusyI2C = 0;
		return 0;
	}
	else{
		return 1;
	}
}

//Bloqueante
uint8_t readRegisters(uint8_t slave_id, message_Type * message, uint8_t * regData){
	if(!isBusyI2C){
		isBusyI2C = 1;
		i2c_cfg_t master = {.address=slaves_addrs[slave_id], .mode=RS,
				.cant_bytes_tx = 1, .cant_bytes_rx = message->cant_values};
		master.data[0] = message->reg;

		i2cInit_master(I2C_0, &master, 0, 0x30);
		while(I2CisBusy(I2C_0));

		isBusyI2C = 0;
		return 0;
	}
	else{
		return 1; //esta en uno la comunicacion por I2C, no se pudo leer registro
	}
}

/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
static void PISR_request_XYZ_Data(void){

	static uint8_t flag_isXYZReady = 0;
	static uint8_t dr_status = 0;
	static uint8_t numTicks = TICKS_I2C_POLL;
	if(runPISR){
		//si esta activa la PISR
		if(!numTicks--){
			numTicks = TICKS_I2C_POLL;
			//uint8_t dataCount = get_rx_count_data(I2C_0);
			if(!I2CisBusy(I2C_0)){
				//entra por primera vez para verificar si DR_STATUS indica que se puede leer XYZ
				if(!flag_isXYZReady){
					dr_status = get_rx_data(I2C_0);
					if(dr_status & FXOS_DRS_XYZDR_MASK){
						flag_isXYZReady = 1;
						//inicio pedido por data xyz
						i2c_cfg_t master = {.address=slaveInI2C, .mode = RS,
												.cant_bytes_tx = 1, .cant_bytes_rx = numCoord};
						master.data[0] = FXOS8700CQ_OUT_X_MSB;
						i2cInit_master(I2C_0, &master, 0, 0x30);
					}
					else{
						//aun no hay dato listo
						flag_isXYZReady = 0;
						dr_status = 0;
						runPISR = 0;
						isBusyI2C = 0;
					}
				}
				//entra una vez verificado que hayan datos
				else{
					//uint8_t i = 0;
					//for(i = 0; i < get_rx_count_data(I2C_0); i++){
					//	slaveXYZDataAccelMSB[i] = get_rx_data(I2C_0);
					//}
					slaveXYZDataAccelMSB[0] = get_rx_data(I2C_0);
					slaveXYZDataAccelMSB[1] = get_rx_data(I2C_0);
					slaveXYZDataAccelMSB[2] = get_rx_data(I2C_0);
					slaveXYZDataMagMSB[0] = get_rx_data(I2C_0);
					slaveXYZDataMagMSB[1] = get_rx_data(I2C_0);
					slaveXYZDataMagMSB[2] = get_rx_data(I2C_0);

					//apago PISR, listo las coordenadas
					flag_isXYZReady = 0;
					dr_status = 0;
					runPISR = 0;
					isBusyI2C = 0;
				}
			}
		}
	}
}
