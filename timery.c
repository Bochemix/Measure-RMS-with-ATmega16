/*
 * timery.c
 *
 *  Created on: Apr 9, 2015
 *      Author: juju
 */


#include <avr/io.h>
#include <avr/interrupt.h>

#include "timery.h"



void init_timera()
{
// ustawienie TIMER0
TCCR0 |= (1<<WGM01);				// tryb CTC
//TCCR0 |= (1<<CS02)|(1<<CS00);		// preskaler = 1024
TCCR0 |= (1<<CS01);		// preskaler = 8
//OCR0=(10MHz/8/10kHz)-1 = 124   czyli tykniÄ™cie raz na 1/100 sek.
OCR0 = 124;							// dodatkowy podzial przez 107 (rej. przepelnienia)
TIMSK |= (1<<OCIE0);				// zezwolenie na przerwanie CompareMatch
}

ISR(TIMER0_COMP_vect)
{

	licznik ++;
	flag = 1;
	czas = czas + 1;
}


