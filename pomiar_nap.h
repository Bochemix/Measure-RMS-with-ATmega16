/*
 * pomiar_nap.h
 *
 *  Created on: Apr 9, 2015
 *      Author: juju
 */

#ifndef POMIAR_NAP_H_
#define POMIAR_NAP_H_

#include <avr/io.h>

volatile uint32_t WY_ADC;
volatile uint32_t WY_ADC2;

volatile uint16_t v1;
volatile char V1 [4];
volatile uint16_t v2;
volatile char V2 [4];

volatile uint16_t v11;
volatile char V11 [4];
volatile uint16_t v22;
volatile char V22 [4];

uint16_t pomiar(uint8_t kanal);
void init_adc();


#endif /* POMIAR_NAP_H_ */
