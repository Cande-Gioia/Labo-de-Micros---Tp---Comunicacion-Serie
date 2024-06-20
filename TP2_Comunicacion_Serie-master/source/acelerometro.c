/*
 * acelerometro.c
 *
 *  Created on: 25 sep. 2023
 *      Author: juanm
 */
#include "acelerometro.h"
#include "acell_InterfazI2c.h"
#include "accel_board.h"



static int8_t rolido;
static int8_t cabeceo;
static int16_t xyz_accel[3];
static int16_t xyz_mag[3];

static void PISR_new_data(void);


void acellInit(acell_config * config)
{
	initI2C_FXOS(SLAVE1,HYBRID); //corregir despues
	SysTick_Init();
	SysTick_Add(PISR_new_data);
}


uint8_t getStatusRolidoCabeceo(void)
{
	uint8_t new;
	new = isNewXYZMessage(SLAVE1);
	return new;
}


double getRolido(void)
{
	int16_t x = DATA_MSB_2_INT16(xyz_accel[0]) ;
	int16_t z = DATA_MSB_2_INT16(xyz_accel[2]);
	double ang = Cart2Pol_ang(x, z);
		ang -= 90;
		if(ang > 180){
			ang -= 360;
		}
		else if(ang < -180){
			ang += 360;
		}

	return ang;
}

double getCabeceo(void)
{
	int16_t y = DATA_MSB_2_INT16(xyz_accel[1]);
	int16_t z = DATA_MSB_2_INT16(xyz_accel[2]);
	double ang = Cart2Pol_ang(y, z);
		ang -= 90;
		if(ang > 180){
			ang -= 360;
		}
		else if(ang < -180){
			ang += 360;
		}



	return ang;
}

double getMag(void)
{
	int16_t x = DATA_MSB_2_INT16(xyz_mag[0]);
	int16_t y = DATA_MSB_2_INT16(xyz_mag[1]);
	double ang = Cart2Pol_ang(x, y);
	return ang;
}

double Cart2Pol_ang(int16_t v1, int16_t v2)
{
    double theta;
   /* if (v1 == 0)
    {
        if(v2 > 0)
            theta = 90;
        else if(v2 < 0)
            theta = -90;
        else
            theta = 0;
    }*/
    if(v1<0)
    {
        if(v2 >= 0)
        {
            theta = atan( (double)v2 / (double)v1) * (180.0/PI) + 180;
        }
        else
        {
            theta = atan((double)v2 / (double)v1) * (180.0/PI) - 180;
        }
    }
    else
    {
        theta = atan((double)v2 / (double)v1) * (180.0/PI);
    }
    return theta;
}
static void PISR_new_data(void)
{
	static num_ticks = NUMTICKS;
	if(!(num_ticks--))
	{
		num_ticks = NUMTICKS;
		requestDataXYZ(SLAVE1, &xyz_accel, &xyz_mag);
	}
}
