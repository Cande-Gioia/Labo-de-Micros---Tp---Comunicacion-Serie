#include "Encoder.h"
#include "Systick.h"
#include "gpio.h"
#include "Encoder_config.h"


//Interrupcion en caso de giro del encoder
static void PIN_AB(void);

//Interrupcion en caso de que se haya apretado el boton
static void DRV_ISR(void);


/*******************************************************************************
 * VARIABLE WITH GLOBAL SCOPE
 ******************************************************************************/
//Si las vueltas fueron hacia la derecha es positivo, sino negativo
static int vueltas;
//Cantidad de veces que fue apretado el boton
static int bot;

static int val_prev;



void DRV_Enc_Init (void)
{
	gpioMode(PIN_A, INPUT);
	gpioMode(PIN_B, INPUT);
	gpioMode(BOTON, INPUT);
	gpioIRQ(PIN_A, GPIO_IRQ_MODE_BOTH_EDGES, PIN_AB);
	gpioIRQ(PIN_B, GPIO_IRQ_MODE_BOTH_EDGES, PIN_AB);
	SysTick_Init();
	SysTick_Add(DRV_ISR);
	vueltas = 0;
	bot = 0;
	val_prev = (gpioRead(PIN_A)<<1)+ gpioRead(PIN_B);
}


/*******************************************************************************
 * FUNCIONES A UTILIZAR POR APP
 ******************************************************************************/
int get_vueltas()
{
	int32_t giros = vueltas/ESTADOS_TICK;
	vueltas = 0;
	return giros;
}

int get_paso(){
	if(vueltas >= 4)
		return get_vueltas();
	else
		return 0;
}

int get_boton()
{
	int push = bot;
	bot = 0;
	return push;
}



/*******************************************************************************
 * INTERRUPCIONES
 ******************************************************************************/
void PIN_AB(void)
{
	int val_new = (gpioRead(PIN_A)<<1)+ gpioRead(PIN_B);
	switch(val_new)
	{
		case ESTADO1:
			if(val_prev == ESTADO3)
				vueltas++;			//giro DER
			if(val_prev == ESTADO2)
				vueltas--;			//giro IZQ
			break;
		case ESTADO2:
			if(val_prev == ESTADO1)
				vueltas++;
			if(val_prev == ESTADO4)
				vueltas--;
			break;
		case ESTADO3:
			if(val_prev == ESTADO4)
				vueltas++;
			if(val_prev == ESTADO1)
				vueltas--;
			break;
		case ESTADO4:
			if(val_prev == ESTADO2)
				vueltas++;
			if(val_prev == ESTADO3)
				vueltas--;
			break;
	}
	val_prev = val_new;
}

void DRV_ISR(void)
{

	static int val_prev;
	int val_new = gpioRead(BOTON);

	if((val_new != val_prev) && val_new == BOT_ACTIVE)
	{
		bot++;
	}
	val_prev= val_new;

}
