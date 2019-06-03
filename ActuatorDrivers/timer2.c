/*
 * timer2.c
 * used to count in us
 * Created: 15.03.2018 13:53:39
 *  Author: Johannes
 */ 

#include "timer2.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

static uint32_t cntr;	//counter that is incremented after timer overflow

void initTimer2(){
	// Configure timer with normal mode and no prescaler
	TCCR2A = (1<<CS20);
	// Enable overflow interrupt
	TIMSK2 = (1 << TOIE2);
}
void startTimer2(){
	cntr = 0;
	TCNT2 = 0;
}
uint32_t getTimer2ElapsedUs(){
	return cntr*(1000000UL*256/F_CPU) ;
	//return cntr;
}


ISR(TIMER2_OVF_vect) {
	cntr++;
}