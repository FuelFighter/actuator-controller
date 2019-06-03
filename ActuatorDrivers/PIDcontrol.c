/*
 * PIDcontrol.c
 *
 * Created: 16.03.2018 12:17:46
 *  Author: Johannes
 */ 

#include "PIDcontrol.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#define F_CPU 8000000UL

int16_t k_p = 1;

ISR(TIMER2_OVF_vect) {
	//adcReading=ads_1115_get_reading();
	//updatePID
}

void initPIDcontrol(){
	//set up timer2
	// Configure timer with normal mode
	TCCR2A = (1<<CS22)|(1<<CS21)|(1<<CS20);
	// Enable overflow interrupt
	TIMSK2 = (1 << TOIE2);
}

int16_t updatePID(int16_t x, int16_t x_ref){
	return k_p*(x_ref-x);	
}

void stopPIDcontrol(){
	TCCR2A = 0;
}