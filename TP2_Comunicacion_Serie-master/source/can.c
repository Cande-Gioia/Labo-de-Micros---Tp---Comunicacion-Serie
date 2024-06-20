/***************************************************************************//**
  @file     can.c
  @brief    Driver CAN Protocol
  @author   Grupo 2
 ******************************************************************************/


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "can.h"
#include "port.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define INACTIVE 0b1000
#define ABORT 0b1001
#define DATA 0b1100
#define REMOTE 0b1100
#define TANSWER 0b1110
#define INACTIVE_RX 0b0000
#define PULL_RX 0b0010
#define OVERRUN_RX 0b0110
#define RANSNWER_RX 0b1010
#define EMPTY_RX 0b0100
#define BUSYMASK  0b0001
#define MASK 0b1111
/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static CAN_Type * const can_base[1] = CAN_BASE_PTRS; 

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void canInit(void) {

	SIM->SCGC6 |= SIM_SCGC6_FLEXCAN0_MASK;

	PCR_Type* ptr_pcr_RX;
	PCR_Type* ptr_pcr_TX;
	ptr_pcr_RX = PCR_ADRESS(PORTNUM2PIN(PB,19));
	ptr_pcr_TX = PCR_ADRESS(PORTNUM2PIN(PB,18));

	ptr_pcr_RX -> MUX = PORT_mAlt2;
	ptr_pcr_TX -> MUX = PORT_mAlt2;

	ptr_pcr_RX -> PE = 1;
	ptr_pcr_TX -> PE = 1;

	ptr_pcr_RX -> PS = 1;
	ptr_pcr_TX -> PS = 1;

	CAN_Type* ptr_can;
	ptr_can = can_base[0];

	ptr_can->CTRL1 |= CAN_CTRL1_CLKSRC_MASK;

	uint32_t MCR = 0;

	ptr_can->MCR &= ~CAN_MCR_MDIS_MASK;



	ptr_can->MCR |= CAN_MCR_MAXMB(12) | CAN_MCR_SRXDIS_MASK;

	ptr_can->MCR |= MCR;


	ptr_can->CTRL1 |= CAN_CTRL1_PROPSEG(1) | CAN_CTRL1_PRESDIV(49)
					| CAN_CTRL1_PSEG1(2) | CAN_CTRL1_PSEG2(1) | CAN_CTRL1_RJW(0)
					| CAN_CTRL1_LBUF(0);


	CAN0->CTRL2 |= CAN_CTRL2_MRP_MASK;

	ptr_can->MCR &= ~(CAN_MCR_HALT_MASK);

	while(!(ptr_can->MCR & CAN_MCR_FRZACK_MASK));

}

#define CANT_BYTES_WORD 4

void canTransmit(uint16_t id, uint8_t data[8], uint8_t length, uint8_t id_index) {
	CAN_Type* ptr_can = can_base[0];

	ptr_can->MB[id_index].ID = CAN_ID_STD(id);

	ptr_can->MB[id_index].WORD0 = 0;
	ptr_can->MB[id_index].WORD1 = 0;

	for(int i = 0; i<length; i++) {
		if(i<4){
			ptr_can->MB[id_index].WORD0 |= data[i] << (8*(3-i));
		}
		else
			ptr_can->MB[id_index].WORD1 |= data[i] << (8*(7-i));
	}
	
	ptr_can->MB[id_index].CS = CAN_CS_CODE(DATA) |CAN_CS_DLC(length);

}

void canBuffer_Init(uint16_t id, uint8_t id_index) {

	CAN_Type* ptr_can = can_base[0];
	ptr_can->MB[id_index].ID = 0;
	ptr_can->MB[id_index].ID |= CAN_ID_STD(id);
	ptr_can->MB[id_index].CS &= CAN_CS_CODE_MASK;
	ptr_can->MB[id_index].CS |= CAN_CS_CODE(EMPTY_RX);
}


uint8_t canRecieve( uint8_t* data,  uint8_t id_index)

{

	CAN_Type* ptr_can = can_base[0];
	ptr_can->IFLAG1 &= 0x00000200;
	uint32_t dummy = ptr_can->TIMER;
	if( (ptr_can->MB[id_index].CS & CAN_CS_CODE(BUSYMASK) ) || (ptr_can->MB[id_index].CS & CAN_CS_CODE(MASK)) == CAN_CS_CODE(EMPTY_RX)
			|| (ptr_can->MB[id_index].CS & CAN_CS_CODE(MASK)) == CAN_CS_CODE(INACTIVE) )
	{
		return 0; // está ocupado
	}
	uint8_t dlc =  (ptr_can->MB[id_index].CS & CAN_CS_DLC_MASK )>>(CAN_CS_DLC_SHIFT);

	ptr_can->MB[id_index].CS = CAN_CS_CODE(0b0100);

	if(dlc)
	{

		if(dlc < 8){
			for(int i = 0; i<dlc; i++)
			{
				if(i<4)
				{
					data[i] = ptr_can->MB[id_index].WORD0>>8*(3-i);
				}
				else
				{
					data[i] = ptr_can->MB[id_index].WORD1>>8*(7-i);
				}

			}
		}
		else {
			int a = 0;
			a++;
		}

	}

	return dlc;

}











































