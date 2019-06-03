/*
 * timer16.c
 * This C-file helps to measure time in microseconds. It is designed to be very accurate
 *
 * Created: 10.04.2018 18:17:35
 *  Author: Johannes
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define TIMER1_PRESCALER	8	//Choosing a prescaler value of 8 gives every timer tick a duration of 1us at 8Mhz clock frequency

volatile unsigned long T1_overflow_count = 0; 

//Interrupt Service Routine (ISR) for when Timer2's counter overflows; this will occur every 128us
ISR(TIMER1_OVF_vect) //Timer1's counter has overflowed
{
	
	T1_overflow_count++; //increment the timer2 overflow counter
}

//Configure Timer1
void init_timer16()
{
	TCCR1A = 0;	//Normal mode, count to 0hFFFF
	// set prescaler
	switch(TIMER1_PRESCALER){
		case 1:		TCCR1B |= 0b001; break;
		case 8:		TCCR1B |= 0b010; break;
		case 64:	TCCR1B |= 0b011; break;
		case 256:	TCCR1B |= 0b100; break;
		case 1024:	TCCR1B |= 0b101; break;
	}
	//Enable Timer1 overflow interrupt
	TIMSK1 = 1<<TOIE1; //enable Timer2 overflow interrupt. (by making the right-most bit in TIMSK2 a 1)
}

//get the time in microseconds, as determined by Timer1; the precision will be 1 microseconds
unsigned long get_T1_micros()
{
	return T1_overflow_count*65536 + TCNT1;
}

//reset Timer1's counters. It is recommended to prohibit execution of interrupts during execution of this function
void reset_T1()
{
	T1_overflow_count = 0; //reset overflow counter
	TCNT1 = 0; //reset Timer1 counter
	TIFR1 |= 0b1; //reset Timer1 overflow flag; see datasheet pg. 160; this prevents an immediate execution of Timer1's overflow ISR
}

