/*
 * timer.c
 * This C-file helps to either measure time between start and stop or sets a flag after a predetermined timer cycle count.
 * Created: 01/04/17 15:46:48
 *  Author: Sondre
 */ 

#include "timer.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>
#define F_CPU 8000000UL

//Choose prescaler between 1,8,64,256,1024
#define TIMER_PRESCALER 64

#define NUMBER_OF_TIMERS 8
static bool timer_enabled[NUMBER_OF_TIMERS];
static int32_t elapsed_microseconds[NUMBER_OF_TIMERS];

#define NUMBER_OF_TASKS 8
static bool task_enabled[NUMBER_OF_TASKS];
static bool task_flag[NUMBER_OF_TASKS];			//is set to 1 after cycle_count has reached task_interval
static uint16_t cycle_count[NUMBER_OF_TASKS];	//current number of interrupt cycles
static uint16_t task_interval[NUMBER_OF_TASKS];	//number of interrupt cycles after which the task-flag is set

void timer_init() {
	// Configure timer with normal mode
	TCCR0A = 0;
	// Enable overflow interrupt
	TIMSK0 = (1 << TOIE0);
	// set prescaler
	switch(TIMER_PRESCALER){
		case 1:		TCCR0A |= 0b001; break;
		case 8:		TCCR0A |= 0b010; break;
		case 64:	TCCR0A |= 0b011; break;
		case 256:	TCCR0A |= 0b100; break;
		case 1024:	TCCR0A |= 0b101; break;
	}
}

void timer_start(timer_t timer) {
	elapsed_microseconds[timer] = -(1000000LL * TIMER_PRESCALER * TCNT0) / F_CPU ;
	//elapsed_microseconds[timer] = 0;
	timer_enabled[timer] = true;
}

void timer_stop(timer_t timer) {
	timer_enabled[timer] = false;
}

uint16_t timer_elapsed_ms(timer_t timer) {
	return (elapsed_microseconds[timer] + (1000000LL * TIMER_PRESCALER * TCNT0) / F_CPU)/ 1000;
	//return elapsed_microseconds[timer]/1000;
}

uint32_t timer_elapsed_us(timer_t timer) {
	return (elapsed_microseconds[timer] + (1000000LL * TIMER_PRESCALER * TCNT0) / F_CPU);
	//return elapsed_microseconds[timer];
}

//Start a task with the designator task and a task interval of ti
void task_start(uint8_t task, uint16_t ti){
	task_interval[task] = ti;
	task_enabled[task] = true;
	cycle_count[task] = 0;
}

void task_stop(uint8_t task){
	task_enabled[task] = false;
	cycle_count[task] = 0;
}

bool task_is_due(uint8_t task){
	return task_flag[task];
}

void task_is_done(uint8_t task){
	task_flag[task] = 0;
}

ISR(TIMER0_OVF_vect) {
	for (int t = 0; t < NUMBER_OF_TASKS; t++) {
		if (task_enabled[t]){
			cycle_count[t]++;
			if(cycle_count[t] == task_interval[t])	{
				cycle_count[t] = 0;
				task_flag[t] = true;
			}
		}
	}
		
	for (int t = 0; t < NUMBER_OF_TIMERS; t++) {
		if (timer_enabled[t]){
			elapsed_microseconds[t] += (1000000ULL * TIMER_PRESCALER * 256) / F_CPU;
		}
	}
}
