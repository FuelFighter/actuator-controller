/*
 * ads_1115.c
 *
 * Created: 12/22/17 10:56:18 PM
 *  Author: Sivert
 */ 

#include <stdlib.h>
#include "ads_1115.h"
#include "i2c_master.h"


int16_t ads_1115_get_reading(void){
	i2c_start(ADS1115_WRITE);
	i2c_write(0b00000000);
	i2c_stop();
	i2c_start(ADS1115_READ);
	int16_t msb= i2c_read_ack();//MSB
	int16_t lsb= i2c_read_nack();//LSB
	i2c_stop();
	int16_t data = (msb << 8 | lsb);
	return data;
}

int ads1115_ready(void){
	i2c_start(ADS1115_WRITE);
	i2c_write(0b00000001);
	i2c_stop();
	i2c_start(ADS1115_READ);
	int16_t msb= i2c_read_ack();//MSB
	int16_t lsb= i2c_read_nack();//LSB
	i2c_stop();
	int data = msb>>7;
	return data;
}

int16_t ads_1115_write_config(void){
	i2c_start(ADS1115_WRITE);
	i2c_write((int8_t)0b00000001);//Register: config
	i2c_write((int8_t)0b01000000);//MSB
	i2c_write((int8_t)0b10100000);//LSB   101 = 250 sps
	i2c_stop();
	return 1;
}