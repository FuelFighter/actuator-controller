/*
 * encoder.h
 *
 * Created: 15.03.2018 21:37:13
 *  Author: Johannes
 */ 


#ifndef ENCODER_H_
#define ENCODER_H_

#include <stdint.h>

void initEncoder();

int16_t getEncoderAngle();

uint32_t getEncoderSpeed16();
uint32_t getEncoderSpeed();


int8_t getEncoderDirection();
void setEncoderDirection(int8_t);

#endif /* ENCODER_H_ */