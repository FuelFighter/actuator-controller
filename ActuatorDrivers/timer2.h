/*
 * timer2.h
 *
 * Created: 01/04/17 15:46:57
 *  Author: Sondre
 */ 

#include <avr/io.h>
static uint32_t cntr;
void initTimer2();

void startTimer2();

void stopTimer2();

uint32_t getTimer2ElapsedUs();

