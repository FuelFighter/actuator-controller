/*
 * encoder.c
 * This file helps to read position and speed of an AB-encoder. A has to be connected to an interrupt pin. B can be 
 * connected to any I/O pin
 *
 * Created: 15.03.2018 21:35:36
 *  Author: Johannes
 */ 

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "../UniversalModuleDrivers/timer.h"
#include "../UniversalModuleDrivers/timer16.h"

// define timer used to calculate speed. This is not the hardware timer of the AVR. See UniversalModuleDrivers.timer
#define TimerEncoder	3
#define CPR 500	// Counts per revolution of the Encoder used

volatile uint32_t dt16 = 0;		// time passed in us during one revolution
volatile uint32_t dt = 0;		// time passed in us during one revolution
volatile int16_t encoderCounter = 0;
volatile int8_t encoderDirection = 1;	// 1 for positive direction, -1 for negative direction


// Interrupt on encoder pin A
ISR(INT2_vect){
	cli();
/*

	if (PIND & (1<<PD3)){
		encoderCounter++;
		encoderDirection = 1;
	}
	else{
		encoderCounter--;
		encoderDirection = -1;
	}*/
	encoderDirection = 1;
	encoderCounter++;
	
	if(encoderCounter == CPR || encoderCounter == -CPR){
		cli();
		dt16 = get_T1_micros();
		reset_T1();


		dt = timer_elapsed_us(TimerEncoder);
		timer_start(TimerEncoder);
		sei();
		encoderCounter = 0;
	}
	sei();
}

void initEncoder(){
	//initialize timer

	timer_init();
	timer_start(TimerEncoder);
	init_timer16();
	
	//Enable level-change-interrupt on pin
	EICRA |= (1<<ISC21)|(1<<ISC20);	// rising edge on INT2 (PD2) generates an interrupt
	EIMSK |= (1<<INT2);	// Enable interrupt on INT2
}

int16_t getEncoderAngle(){
	return ((int32_t)encoderCounter*360)/CPR;
}

int8_t getEncoderDirection(){
	return encoderDirection;
}
//returns speed measured using 16 bit timer 1
int32_t getEncoderSpeed16(){
		return encoderDirection*60*1000000LL/dt16;
}
//returns speed measured using 8 bit timer 0
int32_t getEncoderSpeed(){
		return encoderDirection*60*1000000LL/dt;
}

void setEncoderDirection(int8_t newDir){
	encoderDirection = newDir;
}