/*******************************************************************************
  @file     +acelerometro.h+
  @brief    +Driver para I2C del FXOS8700CQ+
  @author   +GRUPO 2+
 ******************************************************************************/
#ifndef ACELEROMETRO_H_
#define ACELEROMETRO_H_


/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h>
#include <math.h>

typedef struct {
	uint8_t nbitPresicion;
	uint8_t magnetometroEnable;

}acell_config;

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 **
 ******************************************************************************/
/*
 * brief: Inicializa Driver
 * params: + magEnable: 1 si se usa magnetometro, 0 sino se usa
 *
 */
void acellInit(acell_config * config);


/*
 * brief:
 * return:
 */
uint8_t getStatusRolidoCabeceo(void);

/*
 * brief: Segun los datos de accel, calcula rolido
 * return: angulo de el rolido de -180 a 180
 */
double getRolido(void);

/*
 * brief: Segun los datos de accel, calcula cabeceo
 * return: angulo de cabeceo de -180 a 180
 */
double getCabeceo(void);


/*
 * brief: Segun los datos de magnetometro, calcula la orientacion
 * return: angulo de orientacion de -180 a 180
 */
double getMag(void);

//devuelve el angulo de inclinacion con respecto a uno de los ejes
double Cart2Pol_ang(int16_t v1, int16_t v2);



#endif /* ACELEROMETRO_H_ */
