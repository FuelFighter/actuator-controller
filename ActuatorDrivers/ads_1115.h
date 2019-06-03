/*
 * ads_1115.h
 *
 * Created: 12/22/17 10:55:44 PM
 *  Author: Sivert
 */ 

#ifndef ADS_1115_H_
#define ADS1115_H_
#include <stdint.h>

/*
#define ADS1115_WRITE 0b10010100  
#define ADS1115_READ  0b10010101*/

#define ADS1115_WRITE 0b10010000
#define ADS1115_READ  0b10010001
                        

int16_t ads_1115_get_reading(void);
int16_t ads_1115_write_config(void);
int ads1115_ready(void);

#endif