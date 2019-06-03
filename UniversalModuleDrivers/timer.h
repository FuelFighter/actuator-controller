/*
 * timer.h
 *
 * Created: 01/04/17 15:46:57
 *  Author: Sondre
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	TIMER0 = 0,
	TIMER1,
	TIMER2,
	TIMER3,
	TIMER4,
	TIMER5,
	TIMER6,
	TIMER7
} timer_t;

void timer_init();

void timer_start(timer_t timer);

void timer_stop(timer_t timer);

uint16_t timer_elapsed_ms(timer_t timer);

uint32_t timer_elapsed_us(timer_t timer);

void task_start(uint8_t task, uint16_t cycleCount);

void task_stop(uint8_t task);

bool task_is_due(uint8_t task);

void task_is_done(uint8_t task);

#endif /* TIMER_H_ */