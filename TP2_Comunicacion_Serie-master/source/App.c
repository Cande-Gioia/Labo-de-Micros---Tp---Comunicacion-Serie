/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Grupo 2
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"
#include "board.h"
#include "can.h"
#include "timer.h"
#include "acelerometro.h"
#include "uart.h"
#include "acell_InterfazI2C.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define GRUPO2 0x102
enum tipo {R, C, O};

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef struct {
	int16_t rolido;
	int16_t cabeceo;
	int16_t orientacion;
}dataType;

#define MAG_EN 1
#define G0 0x100
#define G1 0x101
#define G2 0x102
#define G3 0x103
#define G4 0x104
#define G5 0x105
#define G6 0x106

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/
static uint8_t ids_buff[6];
static uint16_t IDS[6]= {G0, G1, G3, G4, G5, G6};

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
/*
 * brief: realiza el envio de datos por UART de los valores propios leidos del
 * 			accel
 * params: 	+ tipo: indica si se manda rolido, orientacion o cabeceo, segun
 * 				enum tipo
 * 			+ val: valor en entero signado a mandar por UART
 */
static void uart_send(uint8_t tipo, int16_t val);

/*
 * brief: realiza el envio de datos por CAN de los valores propios leidos del
 * 			accel
 * params: 	+ tipo: indica si se manda rolido, orientacion o cabeceo, segun
 * 				enum tipo
 * 			+ val: valor en entero signado a mandar por CAN
 */
static void can_send(uint8_t tipo, int16_t val);

/*
 * brief: convierte datos enteros signados a un arreglo de char
 * params: 	+ tipo: indica si se manda rolido, orientacion o cabeceo, segun
 * 				enum tipo
 * 			+ val: valor en entero signado a convertir
 * 			+ data_to_send: puntero al arreglo donde se guardan los datos
 */
static void DatatoChar(uint8_t tipo, int16_t val, unsigned char* data_to_send);


/*******************************************************************************
 *******************************************************************************
 *                      GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 *******************************************************************************/
void App_Init (void) {
	uart_cfg_t config;
	config.baudrate = 460800;
	config.parity = 0;					//arreglar estos parametros despues


	canInit();
	uartInit(UART_0, config);		//por los 50ms pero checkear despues

	//Inicializacion de buffer para recibir can
	ids_buff[0] = 9;
	ids_buff[1] = 3;
	ids_buff[2] = 4;
	ids_buff[3] = 6;
	ids_buff[4] = 7;
	ids_buff[5] = 8;

	canBuffer_Init(G0,9);
	canBuffer_Init(G1,3);
	canBuffer_Init(G3,4);
	canBuffer_Init(G4,6);
	canBuffer_Init(G5,7);
	canBuffer_Init(G6,8);


	//inicializacion de timers con resolucion de 5ms
	timerInit(5);


	gpioMode(PORTNUM2PIN(PE,26), OUTPUT);
	gpioWrite(PORTNUM2PIN(PE,26), 1);
}


/* Función que se llama constantemente en un ciclo infinito */

void App_Run (void) {

	//Inicializa accel
	acell_config acell_conf;
		acell_conf.nbitPresicion = 1;		//ver que va aca
		acell_conf.magnetometroEnable = 1;
		acellInit(&acell_conf);

//	uint8_t status = acellInit(MAG_EN);
	dataType data_old = {0, 0, 0};
	dataType data_new;
	bool time_passed_50ms = true;

	uint8_t timer_on[3] = {0, 0, 0}; //para saber si se inicio el timer del rolido, cabeceo o el de orientacion respectivamente

	uint8_t time_passed[3];

	while (1)
	{
		data_new.rolido = getRolido();
		data_new.cabeceo = getCabeceo();
		data_new.orientacion = getMag();

		int16_t arr_data[3] = {data_new.rolido, data_new.cabeceo, data_new.orientacion};
		int16_t rol = data_new.rolido - data_old.rolido;
		int16_t cab = data_new.cabeceo - data_old.cabeceo;
		int16_t ori = data_new.orientacion - data_old.orientacion;

		int16_t dif[3] = {rol, cab, ori};

		for(int i=0; i<3; i++){
			if(timePassed(i)){
				time_passed[i] = 1;
			}
		}

		for(int i=0; i<3; i++)
		{
			//Si pasa el tiempo de 50ms y cambianda datos, entonces manda
			//Si pasan 2 segundos sin variacion tambien manda datos
			if((time_passed[i]) || ((time_passed_50ms) && (dif[i] > 5 || dif[i] <-5)))  //si pasaron los 2s o (pasaron 50ms y cambio el angulo)
			{
				time_passed_50ms = false;
				uart_send(i, arr_data[i]);
				can_send(i, arr_data[i]);
				timer_on[i] = 0;
				setTimeAndInit(4,50);
				time_passed[i] = 0;
				gpioToggle(PORTNUM2PIN(PE,26));
			}
			else if(!timer_on[i])
			{
				timer_on[i] = 1;
				setTimeAndInit(i,2000);
			}
		}
		if(timePassed(4)){
			time_passed_50ms = true;
		}


		data_old.rolido = data_new.rolido;
		data_old.cabeceo = data_new.cabeceo;
		data_old.orientacion = data_new.orientacion;


		uint8_t temp_data[8] = {0};

		//Se realiza el polling de Bus Can
		for(int i=0; i<6; i++)
		{
			uint8_t dlc = canRecieve( temp_data,  ids_buff[i]);
			if(dlc)
			{
				if(i==0){
					int b=0;
				}
				unsigned char msg[dlc+1];
				uint8_t g_num = (IDS[i] - G0);
				msg[0] = (unsigned char) g_num + '0' ;
				int k = 0;
				for(int j = 1; j <= dlc ; j++)
				{
					msg[j] = (unsigned char) temp_data[k++];
				}
				uartWriteMsg(UART_0, (unsigned const char*)msg, dlc+1);
			}
		}
	}
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
static void uart_send(uint8_t tipo, int16_t val)
{
	unsigned char data_to_send[5] = {0};
	DatatoChar( tipo,  val, data_to_send);
	unsigned char msg[6];
	msg[0] = '2';
	for(int i=0; i<5;i++){
		msg[i+1] = data_to_send[i];
	}
	uartWriteMsg(UART_0, msg, 6);
}

static void can_send(uint8_t tipo, int16_t val)
{
	unsigned char data_to_send[8] = {0};
	DatatoChar( tipo,  val, data_to_send);
	switch (tipo){
		case R:
			canTransmit(G2, data_to_send, 5 , 11);
		break;
		case C:
			canTransmit(G2, data_to_send, 5 , 12);
		break;
		case O:
			canTransmit(G2, data_to_send, 5 , 14);
		break;
	}
}
static void DatatoChar(uint8_t tipo, int16_t val, unsigned char* data_to_send)
{
	switch(tipo)
	{
	case R:
		data_to_send[4] = val>0 ? ((unsigned char)(val%10)) + '0' : ((unsigned char)((-1)*val%10)) + '0' ;
		data_to_send[3] = val>0 ? ((unsigned char)((val/10)%10)) + '0' : ((unsigned char)((-1)*(val/10)%10)) + '0' ;
		data_to_send[2]  =val>0 ? ((unsigned char)((val/100)%10)) + '0' : ((unsigned char)((-1)*(val/100)%10)) + '0' ;
		data_to_send[1] = val>0 ? '+' : '-';
		data_to_send[0] = 'R';
	break;
	case O:
		data_to_send[4] = val>0 ? ((unsigned char)(val%10)) + '0' : ((unsigned char)((-1)*val%10)) + '0' ;
		data_to_send[3] = val>0 ? ((unsigned char)((val/10)%10)) + '0' : ((unsigned char)((-1)*(val/10)%10)) + '0' ;
		data_to_send[2]  =val>0 ? ((unsigned char)((val/100)%10)) + '0' : ((unsigned char)((-1)*(val/100)%10)) + '0' ;
		data_to_send[1] = val>0 ? '+' : '-';
		data_to_send[0] = 'O';
		break;
	case C:
		data_to_send[4] = val>0 ? ((unsigned char)(val%10)) + '0' : ((unsigned char)((-1)*val%10)) + '0' ;
		data_to_send[3] = val>0 ? ((unsigned char)((val/10)%10)) + '0' : ((unsigned char)((-1)*(val/10)%10)) + '0' ;
		data_to_send[2]  =val>0 ? ((unsigned char)((val/100)%10)) + '0' : ((unsigned char)((-1)*(val/100)%10)) + '0' ;
		data_to_send[1] = val>0 ? '+' : '-';
		data_to_send[0] = 'C';
		break;
	}
}



