#include "pomiar_nap.h"

		uint16_t pomiar(uint8_t channel)		// function for measure voltage
		{
			ADMUX = (ADMUX & 0xF8) | channel;
			ADCSRA |= (1<<ADSC);
			while(ADCSRA & (1<<ADSC));
			printf ("%ud  \n\r",TCNT1);
			return ADCH;
		}
		void init_adc()
		{
			ADMUX	|=	(1<<REFS0);
			ADCSRA	|=	(1<<ADEN)|(1<<ADPS1)|(1<<ADPS0);
		}
