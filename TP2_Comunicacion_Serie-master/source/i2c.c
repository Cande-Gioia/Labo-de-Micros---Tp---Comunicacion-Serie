/***************************************************************************//**
  @file     i2c.c
  @brief    Driver CAN Protocol
  @author   Grupo 2
 ******************************************************************************/


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/


#include "i2c.h"
#include "port.h"
#include "cola.h"
#include "gpio.h"
#include "board.h"




/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static I2C_Type * const i2c_base[I2C_CANT_IDS] = I2C_BASE_PTRS;

static pinIrqFun_t IQRs[I2C_CANT_IDS];

static uint32_t startCount[I2C_CANT_IDS];

static bool address_sent_flag;

static cola_t colaI2CTx[I2C_CANT_IDS];
static cola_t colaI2CRx[I2C_CANT_IDS];
static i2c_cfg_t config_master[I2C_CANT_IDS];
static index_data;
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
static void masterIRQ(uint32_t i2c);
static void slaveIRQ(uint32_t i2c);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void i2cInit (uint8_t id) {
	PCR_Type* ptr_pcr_slc;
	PCR_Type* ptr_pcr_sda;


	switch(id)
	{
	case I2C_0:
		SIM->SCGC4 |= SIM_SCGC4_I2C0(1);
		NVIC_EnableIRQ(I2C0_IRQn);
		ptr_pcr_slc = PCR_ADRESS(PORTNUM2PIN(PE,24));
		ptr_pcr_sda = PCR_ADRESS(PORTNUM2PIN(PE,25));

		ptr_pcr_slc -> MUX = PORT_mAlt5;
		ptr_pcr_sda -> MUX = PORT_mAlt5;
		ptr_pcr_slc -> ODE = 1;
		ptr_pcr_sda -> ODE = 1;

		break;
	case I2C_1:

		break;
	case I2C_2:

		break;
	}
	colaInit(&colaI2CTx[id]);
	colaInit(&colaI2CRx[id]);

}

//revisar mul e icr
void i2cInit_master (uint8_t id, i2c_cfg_t* config, uint8_t mul, uint8_t icr) {
	I2C_Type* ptr_i2c;
	i2cInit (id);

	ptr_i2c = i2c_base[id];
	config_master[id] = *config;
	ptr_i2c->F=0;
	ptr_i2c->F |= I2C_F_MULT(mul);
	ptr_i2c->F |= I2C_F_ICR(icr);
	ptr_i2c->C1 = 0;
	ptr_i2c->C1 |= I2C_C1_IICEN(1);
	ptr_i2c->C1 |= I2C_C1_IICIE(1);

	startCount[id] = 0;
	ptr_i2c->C1 &= ~I2C_C1_TXAK_MASK;
	ptr_i2c->C1 |= I2C_C1_MST(1); //start signal

	set_tx_rx(TX, id);
	set_adress_slave(config->address, id, config->mode);
	index_data = 0;
}


void set_adress_slave(uint8_t address, uint8_t id, uint8_t tx_rx) {
	I2C_Type* ptr_i2c;
	ptr_i2c = i2c_base[id];
	switch(tx_rx) {
	case TX:
	case RS:
		ptr_i2c->D = (address<<1);
		break;
	case RX:
		ptr_i2c->D = (address<<1) + 1;

		if(config_master[id].cant_bytes_rx==1) {
			i2c_base[id]->C1 |= I2C_C1_TXAK(1);
		}
		break;
	}
}

void set_tx_rx(uint8_t tx_rx, uint8_t id) {
	I2C_Type* ptr_i2c;
	ptr_i2c = i2c_base[id];
	switch(tx_rx) {
	case TX:
	case RS:
		ptr_i2c->C1 |= I2C_C1_TX(1);
		break;
	case RX:
		ptr_i2c->C1 &= ~I2C_C1_TX_MASK;
		break;
	}

}


void i2cInit_slave (uint8_t id, i2c_cfg_t config, uint8_t mul, uint8_t icr, uint8_t tx_rx, uint8_t address) {
	I2C_Type* ptr_i2c;
	i2cInit (id);
	ptr_i2c = i2c_base[id];
	ptr_i2c->C2 &= ~I2C_C2_GCAEN_MASK;
	ptr_i2c->C2 &= ~I2C_C2_ADEXT_MASK;
	ptr_i2c->A1 |= I2C_A1_AD(address);
	ptr_i2c->C1 |= I2C_C1_IICEN(1);
	ptr_i2c->C1 |= I2C_C1_IICIE(1);
	ptr_i2c->C1 &= ~I2C_C1_MST_MASK;
	index_data = 0;

}

uint8_t get_rx_data(uint8_t id){
	return colaPull(&colaI2CRx[id]);
}

uint8_t get_rx_count_data(uint8_t id){
	return colaI2CRx[id].count;
}

bool I2CisBusy(uint8_t id) {
    if(!(i2c_base[id]->S & I2C_S_BUSY_MASK))
        return false;
    else
        return true;
}



static void I2C_IQR_handler(uint32_t i2c) {
	//Chequear filtro de glitch
	if((i2c_base[i2c]->FLT & I2C_FLT_STOPF_MASK) == I2C_FLT_STOPF(1)) {
		i2c_base[i2c]->FLT |= I2C_FLT_STOPF(1);
		return;
	}
	else {
		if((i2c_base[i2c]->FLT & I2C_FLT_STARTF_MASK)== I2C_FLT_STARTF(1)) {
			i2c_base[i2c]->FLT |= I2C_FLT_STARTF(1);
			if(!startCount[i2c])
				startCount[i2c]++;
			else
				return;
		}
	}
	//Limpio interrupción
	i2c_base[i2c]->S |= I2C_S_IICIF(1);

	//Modo master o slave
	if((i2c_base[i2c]->C1 & I2C_C1_MST_MASK)== I2C_C1_MST(1))
		masterIRQ(i2c);
	else
		slaveIRQ(i2c);
}

static void masterIRQ(uint32_t i2c) {
	
	static uint8_t bytes_read = 0;

	if((i2c_base[i2c]->C1 & I2C_C1_TX_MASK)== I2C_C1_TX(1)) {
		//Si termine de transmitir o no recibi el ACK
		if(i2c_base[i2c]->S == I2C_S_RXAK(1))
			i2c_base[i2c]->C1 &= ~I2C_C1_MST_MASK;

			if(config_master[i2c].mode == TX)
			{
				if (config_master[i2c].cant_bytes_tx == index_data)
					i2c_base[i2c]->C1 &= ~I2C_C1_MST_MASK;
				else {
					i2c_base[i2c]->D = config_master[i2c].data[index_data] ;

					index_data++;
				}

			}

			else if(config_master[i2c].mode == RX)
			{
				i2c_base[i2c]->C1 &= ~I2C_C1_TX_MASK;
				uint8_t dummy = i2c_base[i2c]->D;
			}

			else if(config_master[i2c].mode == RS)
			{

				if (config_master[i2c].cant_bytes_tx == index_data) {

					i2c_base[i2c]->C1 |= I2C_C1_RSTA(1);
	
					config_master[i2c].mode = RX;
					set_adress_slave(config_master[i2c].address, i2c, config_master[i2c].mode);


				}
				else {
				i2c_base[i2c]->D = config_master[i2c].data[index_data];
				index_data++;
				}

			}
	
	}
	//Rx
	else if(((i2c_base[i2c]->C1 & I2C_C1_TX_MASK) == 0) && ((i2c_base[i2c]->S & I2C_S_TCF_MASK) == I2C_S_TCF_MASK)){
		if(bytes_read == config_master[i2c].cant_bytes_rx-1) //último byte a leer
		{
			i2c_base[i2c]->C1 &= ~I2C_C1_MST_MASK;

			bytes_read = 0;

		}
		else if (bytes_read == config_master[i2c].cant_bytes_rx-2)  //anteúltimo byte a leer
		{
			i2c_base[i2c]->C1 |= I2C_C1_TXAK(1);
			bytes_read++;
		}
		else
		{
			bytes_read++;
		}
		uint8_t a = i2c_base[i2c]->D;
		colaPush(&colaI2CRx[i2c], a);


	}
}


static void slaveIRQ(uint32_t i2c) {
	bool flagIASS = false;


	//arbitration loss
	if(i2c_base[i2c]->S == I2C_S_ARBL(1)) {
		i2c_base[i2c]->S = I2C_S_ARBL(1);
		if(i2c_base[i2c]->S == I2C_S_IAAS(1))
			flagIASS = true;
		else
			return;
	}

	else if(i2c_base[i2c]->S == I2C_S_IAAS(1))
		flagIASS = true;

	else {
		//Tx
		if(i2c_base[i2c]->C1 == I2C_C1_TX(1)) {
			if(i2c_base[i2c]->S == I2C_S_RXAK(0))
				i2c_base[i2c]->D = colaPull(&colaI2CTx[i2c]);
			
		}
		//Rx
		else
			colaPush(&colaI2CRx[i2c], i2c_base[i2c]->D);

	}

	if(flagIASS) {
		if(i2c_base[i2c]->S == I2C_S_SRW(1)) {
			i2c_base[i2c]->C1 = I2C_C1_TX(1);
			i2c_base[i2c]->D = colaPull(&colaI2CTx[i2c]);
		}
		else {
			i2c_base[i2c]->C1 = I2C_C1_TX(0);
			uint8_t dummy = i2c_base[i2c]->D;
		}
	}
}




__ISR__ I2C0_IRQHandler(void) {
	I2C_IQR_handler(I2C_0);
}

__ISR__ I2C1_IRQHandler(void) {
	I2C_IQR_handler(I2C_1);
}

__ISR__ I2C2_IRQHandler(void) {
	I2C_IQR_handler(I2C_2);
}





