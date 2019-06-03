/*
 * actuator.c
 *
 * Created: 12/22/17 10:44:49 PM
 *  Author: Sivert
 */ 


#include <avr/io.h>
#include "actuator.h"



typedef enum {
	ACTUATOR_OUT = (1 << 3),
	ACTUATOR_IN = (1 << 4),
} ActuatorDirection_t;


void actuator_in(){
	PORTE &= ~ACTUATOR_OUT;//UT AV/PÅ
	PORTE |= ACTUATOR_IN;//UT AV/PÅ
}

void actuator_out(){
	PORTE &= ~ACTUATOR_IN;//UT AV/PÅ
	PORTE |= ACTUATOR_OUT;//UT AV/PÅ
}

void actuator_pwmSpeed(uint8_t speed){
	OCR3A=speed;
	OCR3B=speed;
	PORTE &= ~(1<<PE4);
	PORTE &= ~(1<<PE3);
}

