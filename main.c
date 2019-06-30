/*
 * FF:Actuator_2018.c
 *
 * Created: 21/12/17 22:00
 * Author : Johannes Nadler
 */ 

#define ACTUATOR_CAN_ID 0x220		// CAN ID from actuator to controller (transmit) (120 for MC_1/right, 220 for MC_2/left)
#define CONTROLLER_CAN_ID 0x261		// CAN ID from controller to actuator	(receive) (251 for MC_1/right, 261 for MC_2/left)

#define CONSTANT_ERROR_LIMIT 300 // 
#define CONSTANT_ERROR_RESET_LIMIT 200
#define CONSTANT_ERROR_CHANGE_THRESHOLD 10
#define U_NEAR_U0_THRESHOLD 3

#ifndef F_CPU
	#define F_CPU 8000000UL
#endif


//#include <stdlib.h>
//#include <stdint.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "UniversalModuleDrivers/timer.h"
#include "UniversalModuleDrivers/can.h"
#include "UniversalModuleDrivers/rgbled.h"
#include "ActuatorDrivers/i2c_master.h"
#include "ActuatorDrivers/actuator.h"
#include "ActuatorDrivers/ads_1115.h"
#include "ActuatorDrivers/encoder.h"
#include "AVR-UART-lib/usart.h"
#include "UniversalModuleDrivers/timer16.h"

//ACTUATOR CONTROL
#define POSITION_TOLERANCE	50

// TASK IDs
#define TASK_LED	0
#define TASK_UART_READ	1
#define TASK_P_CONTROLLER	2
#define TASK_UART_WRITE 3
#define TASK_CAN_TX	4
#define TASK_CAN_RX	5
#define TASK_SINE 6

// CAN Types
CanMessage_t rxFrame;
CanMessage_t txFrame;

volatile int16_t adcReading = 0;
uint16_t firstGearPosition, idlePosition, secondGearPosition;
uint8_t current_gear;	// current gear the motor is in. 0 = out of gear, 1 = first gear, 2 = second gear
uint8_t reference_gear;		//requested gear (0,1,2)
uint8_t near_gear, closest_gear;
int8_t data;
int16_t adr;
void pwm_start(){
		DDRE	|= (1<<PE3 )|(1<<PE4 );	// use PE3 and PE4 as output to control the H-Bridge driver
		TCCR3B	|=(1<<CS00);  //NO PRESCALER
		TCCR3A	|= (1<<COM3A1)|(1<<COM3B1)|(1<<COM3B0)| (1<<WGM31);	// Fast PWM, Top is ICR3. PE3 and PE4 are inverted
		TCCR3B	|= (1<<WGM32)  |(1<<WGM33);
		ICR3 = 256;	// Switching frequency is F_CPU/ICR3 if no prescaler is used
}
void pwm_stop(){
	TCCR3B = 0;
	TCCR3A = 0;
	PORTE &= ~( (1<<PE3)|(1<<PE4) );	// low level on both h-bridge inputs
}
void setpwm(uint8_t duty){
		OCR3A= duty;
		OCR3B= duty;
}



int main (void)
{	
	pwm_start();	//inits pwm for h bridge control
	rgbled_init();
	rgbled_turn_on(LED_BLUE);
	uint8_t duty = 20;
	int16_t x, x_ref, x_ref_prev, e, u, u0;
	int16_t e_prev = 0;
	uint8_t u_is_u0;
	long constant_error_counter = 0;
	uint8_t pause_counter = 0;
	uint8_t is_stuck = 0;
	u0 = 128;
	float kp = 0.7;
	char msg[22]; // heading, 20 digit bytes, NULL
	
	setpwm(duty);
	i2c_init();
	ads_1115_write_config();
	
	uart_init(BAUD_CALC(500000)); // 8n1 transmission is set as default
	uart_puts("Ei gude wie?");
	initEncoder();	
	can_init(0,0);
	sei();
	
	firstGearPosition = eeprom_read_word((uint16_t*)46);
	secondGearPosition = eeprom_read_word((uint16_t*)44);
	idlePosition = eeprom_read_word((uint16_t*)42);
	x_ref = firstGearPosition;
	x_ref_prev = x_ref;
	near_gear = 2; //assuming this is safest
	closest_gear = near_gear;
	
	task_start(TASK_LED, 1000);	//task 1 is due after 1000 interrupt cycles 
	task_start(TASK_UART_WRITE,50);
	task_start(TASK_UART_READ,50);
	task_start(TASK_CAN_TX, 100);
	task_start(TASK_CAN_RX, 10);
	task_start(TASK_P_CONTROLLER,1);
	

	while(1){
		if(get_T1_micros() > 50000UL){	//set motor speed to zero if no pulse has occurred for a long time
			setEncoderDirection(0);
			
		}
		if (task_is_due(TASK_LED)){
			rgbled_toggle(LED_BLUE);
			if (!is_stuck) {
				rgbled_turn_off(LED_RED);
				rgbled_turn_on(LED_GREEN);
			} else {
				rgbled_turn_off(LED_GREEN);
				rgbled_turn_on(LED_RED);
			}
			task_is_done(TASK_LED);
		}
		if (task_is_due(TASK_UART_WRITE)){
/*
			x=ads_1115_get_reading();
			uart_putint(getEncoderSpeed());
			uart_puts("|");
			uart_putint(getEncoderAngle());
			uart_puts("|");
			uart_putint(x);
			uart_puts("\r\n");*/

						uart_putint(x_ref);
						uart_puts("|");
						uart_putint(e);
						uart_puts("|");
						uart_putint(u);
						uart_puts("|");
						uart_putlong(constant_error_counter);
						uart_puts("|");
						uart_putint(u_is_u0);
						uart_puts("|");
						uart_putint(u != 128);
						uart_puts("|");
						uart_putint(is_stuck);
						uart_puts("\r\n");
						
			task_is_done(TASK_UART_WRITE);
		}
		if (task_is_due(TASK_UART_READ)){
			if(uart_AvailableBytes()!=0){
				
				//uart0_getlnToFirstWhiteSpace(msg, 12);	// reads until first white space
				uart0_getln(msg, 22);						// reads until \r\n
				
				if(strcmp(msg, "release") == 0){
					 pwm_stop();
					 task_stop(TASK_SINE);
					 timer_stop(6);
				}
				if(strcmp(msg, "start") == 0) pwm_start();
				if(strcmp(msg, "setFirstGearPos") == 0) {
					eeprom_write_word ((uint16_t *)46, x);
					firstGearPosition = x;
				}
				if(strcmp(msg, "setSecondGearPos") == 0) {
					eeprom_write_word ((uint16_t *)44, x);
					secondGearPosition = x;
				}
				if(strcmp(msg, "setIdlePos") == 0) {
					eeprom_write_word ((uint16_t *)42, x);
					idlePosition = x;
				}
				if(strcmp(msg, "f") == 0) {
					reference_gear = 1;
					x_ref = firstGearPosition;
				}
				if(strcmp(msg, "s") == 0) {
					reference_gear = 2;
					x_ref = secondGearPosition;
				}
				if(strcmp(msg, "n") == 0) {
					reference_gear = 0;
					x_ref = idlePosition;
				}
				if(strcmp(msg, "sine") == 0) {
					timer_start(6);	
					task_start(TASK_SINE,5);
				}
				if(atoi(msg)!=0){
					reference_gear = 10;		//just an unrealistic gear number
					 x_ref=atoi(msg);
				}
				uart_flush();
			}	
						
			task_is_done(TASK_UART_READ);
		}
		if(task_is_due(TASK_CAN_TX)){
			// Send message
			txFrame.id = ACTUATOR_CAN_ID;
			txFrame.length = 5;
			
			txFrame.data.i16[0] = getEncoderSpeed16();
			txFrame.data.u8[2] = current_gear;
			txFrame.data.u8[3] = closest_gear;
			txFrame.data.u8[4] = near_gear;
			
			
			can_send_message(&txFrame);
			
		task_is_done(TASK_CAN_TX);
		}
		if (task_is_due(TASK_CAN_RX)){
			// Read message
			CanMessage_t rx;
			if (can_read_message_if_new(&rx)){
				adr = rx.id;
				data = rx.data.u8[0];
				if(rx.id == CONTROLLER_CAN_ID) {//receiving can messages from the motor controller
								
					if(rx.data.u8[0]==0){ reference_gear = 0; x_ref = idlePosition;}
					if(rx.data.u8[0]==1){ reference_gear = 1; x_ref = firstGearPosition;}
					if(rx.data.u8[0]==2){ reference_gear = 2; x_ref = secondGearPosition;}
				}
			}
			task_is_done(TASK_CAN_RX);
		}
		if (task_is_due(TASK_P_CONTROLLER)){
			x = ads_1115_get_reading();
			
			if (x > idlePosition + POSITION_TOLERANCE ) {
				closest_gear = 2;
			} else if (x < idlePosition - POSITION_TOLERANCE) {
				closest_gear = 1;
			} else {
				closest_gear = 0;
			}
			
			if (x > ((3/4)*(secondGearPosition-idlePosition) + idlePosition) ) {
				near_gear = 2;
			} else if (x < ((1/2)*(idlePosition - firstGearPosition) + firstGearPosition ))
			{
				near_gear = 1;
			} 
			else
			{
				near_gear = 0;
			}
			
			e = x_ref-x;
			u = kp*e+u0;

			
			if ((abs(e-e_prev) < CONSTANT_ERROR_CHANGE_THRESHOLD) && (!pause_counter)) {
				constant_error_counter++;
			}
			else if ((constant_error_counter > 0) && (!pause_counter)) {
				constant_error_counter--;
			}
			
			if (x_ref != x_ref_prev)
			{
				constant_error_counter = 0;
				pause_counter = 0;
			}
			
			if (constant_error_counter > CONSTANT_ERROR_LIMIT) {
				is_stuck = 1;
			} else if (constant_error_counter < CONSTANT_ERROR_RESET_LIMIT) {
				is_stuck = 0;
				pause_counter = 0;
			}
			
			if(is_stuck) {
				u = u0;
				pause_counter = 1;
			}
			else if(u>255) {
				u = 255;
			}
			else if(u<0) {
				u = 0;
			}
			actuator_pwmSpeed(u);
			//check if target position has been reached
			if(e<POSITION_TOLERANCE && -e<POSITION_TOLERANCE){
				current_gear = reference_gear;
				pause_counter = 1;
			}
			e_prev = e;
			x_ref_prev = x_ref;
			task_is_done(TASK_P_CONTROLLER);
		}
		if(task_is_due(TASK_SINE)){
			x_ref = sin((double)timer_elapsed_ms(6)/500)*8000+10000;
			task_is_done(TASK_SINE);
		}
	}
}