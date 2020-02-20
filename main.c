/*
 * main2.c
 * Program designed to measure RMS of 50 Hz sinewave
 *  Created on: 20 NOV 2019
 *      Author: rafbodz
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// **************** LINKING NECESSARY FILES, LIBRARIES ETC. ***************************

#define F_CPU 11059200UL

#define FOSC 11059200UL// Clock Speed

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>	// for function sqrt()

#include "lcd44780.h" //
#include "timery.h"			//include Timer files
#include "pomiar_nap.h"			//inlcude voltage meter


#define VREF_VCC (1<<REFS0)
#define VREF_256 (1<<REFS1)|(1<<REFS0)	// definine reference voltage

volatile uint16_t counter = 0; // cache variable to navigate
volatile uint8_t FLAG1 = 1; /// FLAG for timer
volatile uint8_t FLAG = 1; // FLAG for ADC
uint8_t AVR_ADC = 0; // mean value of ADC - equals to value of DC voltage
uint8_t x_c = 0;
uint8_t x_r = 0;

// *************************************************************************************

// Variables defined by User
uint8_t sample_number = 200;	// tutaj mozna zdefiniowac ile probek bedzie w wektorze // amount of samples
int16_t sample_vector [200];	// vector of samples

// automatic variables, setting up a program
uint8_t loading_data = 1; // variable responsible for collecting data from ADC
uint8_t i = 0; // variable responsible for navigating on vector
uint16_t sum_ADC = 0; // // variable collecting SUM of values
uint32_t sum_square = 0; // variable collecting SUM * SUM of values
uint32_t sum_square2 = 0; // variable collecting SUM * SUM of values but divided by number of samples
uint32_t sum_square3 = 0; // variable just like sum_square2
uint32_t square1 = 0;
uint8_t debug = 1; // value defining if we want to look into program ( what values are being alocaated into vector)
uint8_t debuger = 2; // just like above, but to see
uint16_t A = 1468; // do not worry about those walues, i will tell you later what is going on
uint16_t B = 287; // just like above
int32_t X = 0;
int8_t x = 0;
int32_t x_temp = 0; // temp values just for proper measurements
int32_t x_temp2 = 0;
int32_t x_temp3 = 0;

 // variables collecting total and decimal part
uint8_t value_RMS = 0;
uint16_t sr_RMS = 0;
uint16_t sr_RMS2 = 0;
uint8_t value_RMS_c = 0;
uint8_t value_RMS_r = 0;
uint8_t average_voltage_wektor_c = 0;
uint8_t average_voltage_wektor_r = 0;

uint8_t average_RMS [5]; //  rolling vector reducing peaks into RMS
uint8_t b = 0;
uint16_t average_RMS_wektor = 0;

int main(void)
{
	lcd_init(); // initiate LCD
// ***************** SETTING TIMER0 *********************************
	TCCR0 |= (1<<WGM01);				// mode CTC
	//TCCR0 |= (1<<CS01)|(1<<CS00);		// preskaler = 64
	TCCR0 |= (1<<CS01);		// preskaler = 8
	//OCR0=(11059200Hz/8) = 1382400
	OCR0 = 139;							// (reg. overflow)
	TIMSK |= (1<<OCIE0);				// allow to CompareMatch
// *********************************************************************

// ***************** SETTING ADC *********************************
	ADMUX = 0;
	ADMUX = (1<<REFS0)|(1<<REFS1)|(1<<ADLAR); // REFS0 i REFS1 - ref. voltage (2,56V),  ADLAR - left shift - 8bit
	ADCSRA = (1<<ADEN)|(1<<ADPS1)|(1<<ADPS0);    // ADEN - allow to use,  ADSC - allow single collect - 11059200 /128 = 86,4kHz
	// ADPS2 ADPS1 ADPS0 -  preskaler for ADC
	// For 10bit ADC frequency should be between 50kHz - 200kHz, but we use 8bit ADC
	// thats why we should set prescaler to 111 = 128
	//ADIE - termination of the voltage conversion switches on the interrupt
// ******************************************************************

	PORTD |= (1<<PD3);
	PORTD &= ~(1<<PD4);

	sei(); // allow to interrupt

	// ******************** HERE WE GO
	lcd_cls(); // clear screen
	lcd_str("WITAMY.."); // welcome text
	_delay_ms(1500); // give some time to see this text
	lcd_cls();

	while(1)
	{
		if ( loading_data == 1) // if conversion is allowed, we measure the voltage
		{
			if ( FLAG == 1 ) // if the FLAG responsible for the ADC conversion is 1 then measurement is carried out
			{
				FLAG = 0;	// we are resetting the FLAG, it will be restored to the value 1 by interruption
				sample_vector[i] = pomiar(0);	// entering the instantaneous voltage value into a specific cell of a vector
				i = i+1;	//navigator increment
			}

			if ( i == sample_number)
			{
				i = 0;	// reset navigator
				loading_data = 0; // the program exits conversion and stops downloading data from the ADC
			}
		}

		if ( loading_data == 0) // right here the program deals with data processing
		{
			if (debug == 1 && debuger == 1)
			{
				lcd_cls();
				lcd_str("POMIAR: "); // the inscription displayed before the measurements are made
				_delay_ms(500); // delay
			}

			if (debug == 1 && debuger == 1)
			{
				 for(uint8_t j = 0; j < sample_number; j=j+20 )  	//the loop shows the values of individual samples
				{												//just to check if everything is okey
					if (j >= sample_number) j = sample_number;
					lcd_locate(1,2);
					lcd_str("  ");
					lcd_locate(1,9);
					lcd_str("   ");

					lcd_locate(1,0);
					lcd_str("j=");
					lcd_locate(1,2);
					lcd_int(j);

					lcd_locate(1,5);
					lcd_str("ADC=");
					lcd_locate(1,9);
					lcd_int(sample_vector[j]);

					_delay_ms(150);
				}
			}

// ******************************* Calculations on data ****************************************************
			for (uint8_t a = 0; a < sample_number; a = a+1) // SUM of all values
			{
				sum_ADC = sum_ADC + sample_vector[a];
			}

			AVR_ADC = sum_ADC/sample_number; // average value

			for (uint8_t a = 0; a < sample_number; a = a+1) // calculation of the signal variable component
			{
				sample_vector[a] = sample_vector[a] - AVR_ADC;
			}

			for (uint8_t b = 0; b < sample_number; b = b+1) // RMS of variable component
			{
				square1 = (uint32_t)(sample_vector[b] * sample_vector[b]);
				sum_square = (uint32_t)(sum_square + square1);
				square1 = 0;
			}

			sum_square3 = (uint32_t)(sum_square);
			sum_square2 = (uint32_t)(sum_square3/sample_number);
			sr_RMS2 = (uint16_t)(sum_square2);
			sr_RMS = (uint16_t)(sqrt(sr_RMS2));
			value_RMS = (uint8_t)(sr_RMS);

			x_temp3 = (int32_t)(sr_RMS);
			x_temp2 = (int32_t)(x_temp3 * 100);
			x_temp = (int32_t)((x_temp2 - B));
			x_temp = x_temp * 100;
			X = (int32_t)(x_temp/A);
			x = (int8_t)(X);

			if (x <= 99){x_c = 0;}else
			if (x > 99 && x <= 199){x_c = 1;}else
			if (x > 199){x_c = 2;}

			if (debug == 1 && debuger == 1)
						{
							 for(uint8_t j = 0; j < sample_number; j=j+20 )  	// the loop shows the values of individual samples
							{												//this part is only to make sure thats everythings okay
								if (j >= sample_number) j = sample_number;
								lcd_locate(1,2);
								lcd_str("  ");
								lcd_locate(1,9);
								lcd_str("   ");

								lcd_locate(1,0);
								lcd_str("X=");
								lcd_locate(1,2);
								lcd_int(j);

								lcd_locate(1,5);
								lcd_str("RMS=");
								lcd_locate(1,9);
								lcd_int(sample_vector[j]);

								_delay_ms(150);
							}

						}

			average_RMS[b] = value_RMS; // enter the rms value into the walking vector
			b=b+1;
			if (b == 5)
			{
				b = 0;
			}

			for (uint8_t a = 0;  a < 5; a=a+1)
			{
				average_RMS_wektor = average_RMS_wektor + average_RMS[a];
			}

			average_RMS_wektor = average_RMS_wektor/5; // div by 5

			if (AVR_ADC <= 99){average_voltage_wektor_c = 0;}else
			if (AVR_ADC > 99 && AVR_ADC <= 199){average_voltage_wektor_c = 1;}else
			if (AVR_ADC > 199){average_voltage_wektor_c = 2;}

			if (average_RMS_wektor <= 99){value_RMS_c = 0;}else
			if (average_RMS_wektor > 99 && average_RMS_wektor <= 199){value_RMS_c = 1;}else
			if (average_RMS_wektor > 199){value_RMS_c = 2;}

			average_voltage_wektor_r = (double)(AVR_ADC % 100);
			value_RMS_r = (double)(average_RMS_wektor % 100);
			x_r = (double)(x % 100);


// ******************************* SHOW ****************************************************

			if (debug == 1)
			{
				lcd_cls();
				lcd_locate(0,0);
				lcd_str("Sr =");

				lcd_locate(0,4);
				lcd_int(average_voltage_wektor_c); // calculation and display of the average of 5 measurements


				lcd_locate(0,5);
				lcd_str(".");
				if (average_voltage_wektor_r < 10)
				{
				lcd_locate(0,6);
				lcd_str("0");
				lcd_locate(0,7);
				lcd_int(average_voltage_wektor_r);
				}else
				{
				lcd_locate(0,6);
				lcd_int(average_voltage_wektor_r);
				}
				lcd_locate(0,8);
				lcd_str("V");

				lcd_locate(1,0);
				lcd_str("RMS= ");

				lcd_locate(1,4);
				lcd_int(value_RMS_c);
				lcd_locate(1,5);
				lcd_str(".");
				if (value_RMS_r < 10)
				{
				lcd_locate(1,6);
				lcd_str("0");
				lcd_locate(1,7);
				lcd_int(value_RMS_r);
				}else
				{
				lcd_locate(1,6);
				lcd_int(value_RMS_r);
				}

				lcd_locate(1,8);
				lcd_str("V");
				//////////////////////////////
				lcd_locate(0,10);
				lcd_int(AVR_ADC);
				lcd_locate(1,10);
				////////////////
				lcd_int(value_RMS);
				_delay_ms(700);

				lcd_locate(0,4);
				lcd_str("   ");
				lcd_locate(1,4);
				lcd_str("   ");
				lcd_locate(0,10);
				lcd_str("   ");
				lcd_locate(1,10);
				lcd_str("     ");
			}

			if (debug == 1 && debuger == 2) // this part is to show the value of electric field strength
			{
					lcd_cls();

					lcd_locate(1,2);
					lcd_str("  ");
					lcd_locate(1,6);
					lcd_str("    ");


					lcd_locate(1,0);
					lcd_str("E=");
					if (x < 0){
						lcd_locate(1,2);
						lcd_str("-");
					}
					lcd_locate(1,3);
					lcd_int(x_c);
					lcd_locate(1,4);
					lcd_str(".");

					if (x < 10)
					{
					lcd_locate(1,5);
					lcd_str("0");
					lcd_locate(1,6);
					lcd_int(x_r);
					}else
					{
					lcd_locate(1,5);
					lcd_int(x_r);
					}

					lcd_locate(1,8);
					lcd_str("kV/m");
					lcd_int(x_r);
					lcd_locate(1,13);
					lcd_int(x);
					lcd_locate(1,12);
					lcd_str(" ");
					_delay_ms(1500);
			}

		X = 0;
		sum_ADC = 0;
		sum_square = 0;
		sr_RMS = 0;
		average_RMS_wektor = 0;
		value_RMS = 0;
		loading_data = 1; // allow to collect data another time
		}
	}
}

