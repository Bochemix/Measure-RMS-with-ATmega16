//-----------------------------------------------------------------------------------------------------------
// *** Obsďż˝uga wyďż˝wietlaczy alfanumerycznych zgodnych z HD44780 ***
// Manual for HD44780 from atnel.pl
// - Sterowanie: tryb 4-bitowy
// - Dowolne przypisanie kaďż˝dego sygnaďż˝u sterujďż˝cego do dowolnego pinu mikrokontrolera
// - Praca z pinem RW podďż˝ďż˝czonym do GND lub do mikrokontrolera (sprawdzanie BusyFLAG - szybkie operacje LCD)
//
// Pliki 			: lcd44780.c , lcd44780.h
// Mikrokontrolery 	: Atmel AVR
// Kompilator 		: avr-gcc
// ďż˝rďż˝dďż˝o 			: http://www.atnel.pl
// Data 			: marzec 2010
// Autor 			: Mirosďż˝aw Kardaďż˝
//----------------------------------------------------------------------------------------------------------
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <util/delay.h>

#include "lcd44780.h"

// makrodefinicje operacji na sygnaďż˝ach sterujďż˝cych RS,RW oraz E

#define SET_RS 	PORT(LCD_RSPORT) |= (1<<LCD_RS)			// stan wysoki na linii RS
#define CLR_RS 	PORT(LCD_RSPORT) &= ~(1<<LCD_RS)		// stan niski na linii RS

#define SET_RW 	PORT(LCD_RWPORT) |= (1<<LCD_RW)			// stan wysoki na RW - odczyt z LCD
#define CLR_RW 	PORT(LCD_RWPORT) &= ~(1<<LCD_RW)		// stan niski na RW - zapis do LCD

#define SET_E 	PORT(LCD_EPORT) |= (1<<LCD_E)			// stan wysoki na linii E
#define CLR_E 	PORT(LCD_EPORT) &= ~(1<<LCD_E)			// stan niski na linii E


//uint8_t check_BF(void);			// deklaracja funkcji wewnďż˝trznej


//********************* FUNKCJE WEWNďż˝TRZNE *********************

//----------------------------------------------------------------------------------------
//
//		 Ustawienie wszystkich 4 linii danych jako WYjďż˝cia
//
//----------------------------------------------------------------------------------------
static inline void data_dir_out(void)
{
	DDR(LCD_D7PORT)	|= (1<<LCD_D7);
	DDR(LCD_D6PORT)	|= (1<<LCD_D6);
	DDR(LCD_D5PORT)	|= (1<<LCD_D5);
	DDR(LCD_D4PORT)	|= (1<<LCD_D4);
}

//----------------------------------------------------------------------------------------
//
//		 Ustawienie wszystkich 4 linii danych jako WEjďż˝cia
//
//----------------------------------------------------------------------------------------
static inline void data_dir_in(void)
{
	DDR(LCD_D7PORT)	&= ~(1<<LCD_D7);
	DDR(LCD_D6PORT)	&= ~(1<<LCD_D6);
	DDR(LCD_D5PORT)	&= ~(1<<LCD_D5);
	DDR(LCD_D4PORT)	&= ~(1<<LCD_D4);
}

//----------------------------------------------------------------------------------------
//
//		 Wysďż˝anie poďż˝ďż˝wki bajtu do LCD (D4..D7)
//
//----------------------------------------------------------------------------------------
static inline void lcd_sendHalf(uint8_t data)
{
	if (data&(1<<0)) PORT(LCD_D4PORT) |= (1<<LCD_D4); else PORT(LCD_D4PORT) &= ~(1<<LCD_D4);
	if (data&(1<<1)) PORT(LCD_D5PORT) |= (1<<LCD_D5); else PORT(LCD_D5PORT) &= ~(1<<LCD_D5);
	if (data&(1<<2)) PORT(LCD_D6PORT) |= (1<<LCD_D6); else PORT(LCD_D6PORT) &= ~(1<<LCD_D6);
	if (data&(1<<3)) PORT(LCD_D7PORT) |= (1<<LCD_D7); else PORT(LCD_D7PORT) &= ~(1<<LCD_D7);
}

#if USE_RW == 1
//----------------------------------------------------------------------------------------
//
//		 Odczyt poďż˝ďż˝wki bajtu z LCD (D4..D7)
//
//----------------------------------------------------------------------------------------
static inline uint8_t lcd_readHalf(void)
{
	uint8_t result=0;

	if(PIN(LCD_D4PORT)&(1<<LCD_D4)) result |= (1<<0);
	if(PIN(LCD_D5PORT)&(1<<LCD_D5)) result |= (1<<1);
	if(PIN(LCD_D6PORT)&(1<<LCD_D6)) result |= (1<<2);
	if(PIN(LCD_D7PORT)&(1<<LCD_D7)) result |= (1<<3);

	return result;
}
#endif

//----------------------------------------------------------------------------------------
//
//		 Zapis bajtu do wyďż˝wietlacza LCD
//
//----------------------------------------------------------------------------------------
void _lcd_write_byte(unsigned char _data)
{
	// Ustawienie pinďż˝w portu LCD D4..D7 jako wyjďż˝cia
	data_dir_out();

#if USE_RW == 1
	CLR_RW;
#endif

	SET_E;
	lcd_sendHalf(_data >> 4);			// wysďż˝anie starszej czďż˝ci bajtu danych D7..D4
	CLR_E;

	SET_E;
	lcd_sendHalf(_data);				// wysďż˝anie mďż˝odszej czďż˝ci bajtu danych D3..D0
	CLR_E;

#if USE_RW == 1
	while( (check_BF() & (1<<7)) );
#else
	_delay_us(120);
#endif

}

#if USE_RW == 1
//----------------------------------------------------------------------------------------
//
//		 Odczyt bajtu z wyďż˝wietlacza LCD
//
//----------------------------------------------------------------------------------------
uint8_t _lcd_read_byte(void)
{
	uint8_t result=0;
	data_dir_in();

	SET_RW;

	SET_E;
	result = (lcd_readHalf() << 4);	// odczyt starszej czďż˝ci bajtu z LCD D7..D4
	CLR_E;

	SET_E;
	result |= lcd_readHalf();			// odczyt mďż˝odszej czďż˝ci bajtu z LCD D3..D0
	CLR_E;

	return result;
}
#endif


#if USE_RW == 1
//----------------------------------------------------------------------------------------
//
//		 Sprawdzenie stanu Busy Flag (Zajďż˝toďż˝ci wyďż˝wietlacza)
//
//----------------------------------------------------------------------------------------
uint8_t check_BF(void)
{
	CLR_RS;
	return _lcd_read_byte();
}
#endif


//----------------------------------------------------------------------------------------
//
//		 Zapis komendy do wyďż˝wietlacza LCD
//
//----------------------------------------------------------------------------------------
void lcd_write_cmd(uint8_t cmd)
{
	CLR_RS;
	_lcd_write_byte(cmd);
}

//----------------------------------------------------------------------------------------
//
//		 Zapis danych do wyďż˝wietlacza LCD
//
//----------------------------------------------------------------------------------------
void lcd_write_data(uint8_t data)
{
	SET_RS;
	_lcd_write_byte(data);
}




//**************************  FUNKCJE PRZEZNACZONE TAKďż˝E DLA INNYCH MODUďż˝ďż˝W  ******************

#if USE_LCD_CHAR == 1
//----------------------------------------------------------------------------------------
//
//		 Wysďż˝anie pojedynczego znaku do wyďż˝wietlacza LCD w postaci argumentu
//
//		 8 wďż˝asnych znakďż˝w zdefiniowanych w CGRAM
//		 wysyďż˝amy za pomocďż˝ kodďż˝w 0x80 do 0x87 zamiast 0x00 do 0x07
//
//----------------------------------------------------------------------------------------
void lcd_char(char c)
{
	lcd_write_data( ( c>=0x80 && c<=0x87 ) ? (c & 0x07) : c);
}
#endif

//----------------------------------------------------------------------------------------
//
//		 Wysďż˝anie stringa do wyďż˝wietlacza LCD z pamiďż˝ci RAM
//
//
//----------------------------------------------------------------------------------------
void lcd_str(char * str)
{
	register char znak;
	while ( (znak=*(str++)) ) lcd_char( znak );
}

#if USE_LCD_STR_P == 1
//----------------------------------------------------------------------------------------
//
//		 Wysďż˝anie stringa do wyďż˝wietlacza LCD z pamiďż˝ci FLASH
//
//----------------------------------------------------------------------------------------
void lcd_str_P(const char * str)
{
	register char znak;
	while ( (znak=pgm_read_byte(str++)) ) lcd_char( znak );
}
#endif


#if USE_LCD_STR_E == 1
//----------------------------------------------------------------------------------------
//
//		 Wysďż˝anie stringa do wyďż˝wietlacza LCD z pamiďż˝ci EEPROM
//
//		 8 wďż˝asnych znakďż˝w zdefiniowanych w CGRAM
//		 wysyďż˝amy za pomocďż˝ kodďż˝w 0x80 do 0x87 zamiast 0x00 do 0x07
//
//----------------------------------------------------------------------------------------
void lcd_str_E(char * str)
{
	register char znak;
	while(1)
	{
		znak=eeprom_read_byte( (uint8_t *)(str++) );
		if(!znak || znak==0xFF) break;
		else lcd_char( znak );
	}
}
#endif


#if USE_LCD_INT == 1
//----------------------------------------------------------------------------------------
//
//		 Wyďż˝wietla liczbďż˝ dziesiďż˝tnďż˝ na wyďż˝wietlaczu LCD
//
//----------------------------------------------------------------------------------------
void lcd_int(int val)
{
	char bufor[17];
	lcd_str( itoa(val, bufor, 10) );
}
#endif

#if USE_LCD_HEX == 1
//----------------------------------------------------------------------------------------
//
//		 Wyďż˝wietla liczbďż˝ szestnastkowďż˝ HEX na wyďż˝wietlaczu LCD
//
//----------------------------------------------------------------------------------------
void lcd_hex(uint32_t val)
{
	char bufor[17];
	lcd_str( ltoa(val, bufor, 16) );
}
#endif

#if USE_LCD_DEFCHAR == 1
//----------------------------------------------------------------------------------------
//
//		Definicja wďż˝asnego znaku na LCD z pamiďż˝ci RAM
//
//		argumenty:
//		nr: 		- kod znaku w pamiďż˝ci CGRAM od 0x80 do 0x87
//		*def_znak:	- wskaďż˝nik do tablicy 7 bajtďż˝w definiujďż˝cych znak
//
//----------------------------------------------------------------------------------------
void lcd_defchar(uint8_t nr, uint8_t *def_znak)
{
	register uint8_t i,c;
	lcd_write_cmd( 64+((nr&0x07)*8) );
	for(i=0;i<8;i++)
	{
		c = *(def_znak++);
		lcd_write_data(c);
	}
}
#endif

#if USE_LCD_DEFCHAR_P == 1
//----------------------------------------------------------------------------------------
//
//		Definicja wďż˝asnego znaku na LCD z pamiďż˝ci FLASH
//
//		argumenty:
//		nr: 		- kod znaku w pamiďż˝ci CGRAM od 0x80 do 0x87
//		*def_znak:	- wskaďż˝nik do tablicy 7 bajtďż˝w definiujďż˝cych znak
//
//----------------------------------------------------------------------------------------
void lcd_defchar_P(uint8_t nr, const uint8_t *def_znak)
{
	register uint8_t i,c;
	lcd_write_cmd( 64+((nr&0x07)*8) );
	for(i=0;i<8;i++)
	{
		c = pgm_read_byte(def_znak++);
		lcd_write_data(c);
	}
}
#endif

#if USE_LCD_DEFCHAR_E == 1
//----------------------------------------------------------------------------------------
//
//		Definicja wďż˝asnego znaku na LCD z pamiďż˝ci EEPROM
//
//		argumenty:
//		nr: 		- kod znaku w pamiďż˝ci CGRAM od 0x80 do 0x87
//		*def_znak:	- wskaďż˝nik do tablicy 7 bajtďż˝w definiujďż˝cych znak
//
//----------------------------------------------------------------------------------------
void lcd_defchar_E(uint8_t nr, uint8_t *def_znak)
{
	register uint8_t i,c;

	lcd_write_cmd( 64+((nr&0x07)*8) );
	for(i=0;i<8;i++)
	{
		c = eeprom_read_byte(def_znak++);
		lcd_write_data(c);
	}
}
#endif


#if USE_LCD_LOCATE == 1
//----------------------------------------------------------------------------------------
//
//		Ustawienie kursora w pozycji Y-wiersz, X-kolumna
//
// 		Y = od 0 do 3
// 		X = od 0 do n
//
//		funkcja dostosowuje automatycznie adresy DDRAM
//		w zaleďż˝noďż˝ci od rodzaju wyďż˝wietlacza (ile posiada wierszy)
//
//----------------------------------------------------------------------------------------
void lcd_locate(uint8_t y, uint8_t x)
{
	switch(y)
	{
		case 0: y = LCD_LINE1; break;

#if (LCD_ROWS>1)
	    case 1: y = LCD_LINE2; break; // adres 1 znaku 2 wiersza
#endif
#if (LCD_ROWS>2)
    	case 2: y = LCD_LINE3; break; // adres 1 znaku 3 wiersza
#endif
#if (LCD_ROWS>3)
    	case 3: y = LCD_LINE4; break; // adres 1 znaku 4 wiersza
#endif
	}

	lcd_write_cmd( (0x80 + y + x) );
}
#endif


//----------------------------------------------------------------------------------------
//
//		Kasowanie ekranu wyďż˝wietlacza
//
//----------------------------------------------------------------------------------------
void lcd_cls(void)
{
	lcd_write_cmd( LCDC_CLS );

	#if USE_RW == 0
		_delay_ms(4.9);
	#endif
}


#if USE_LCD_CURSOR_HOME == 1
//----------------------------------------------------------------------------------------
//
//		Powrďż˝t kursora na poczďż˝tek
//
//----------------------------------------------------------------------------------------
void lcd_home(void)
{
	lcd_write_cmd( LCDC_CLS|LCDC_HOME );

	#if USE_RW == 0
		_delay_ms(4.9);
	#endif
}
#endif

#if USE_LCD_CURSOR_ON == 1
//----------------------------------------------------------------------------------------
//
//		Wďż˝ďż˝czenie kursora na LCD
//
//----------------------------------------------------------------------------------------
void lcd_cursor_on(void)
{
	lcd_write_cmd( LCDC_ONOFF|LCDC_DISPLAYON|LCDC_CURSORON);
}

//----------------------------------------------------------------------------------------
//
//		Wyďż˝ďż˝czenie kursora na LCD
//
//----------------------------------------------------------------------------------------
void lcd_cursor_off(void)
{
	lcd_write_cmd( LCDC_ONOFF|LCDC_DISPLAYON);
}
#endif


#if USE_LCD_CURSOR_BLINK == 1
//----------------------------------------------------------------------------------------
//
//		Wďż˝ďż˝cza miganie kursora na LCD
//
//----------------------------------------------------------------------------------------
void lcd_blink_on(void)
{
	lcd_write_cmd( LCDC_ONOFF|LCDC_DISPLAYON|LCDC_CURSORON|LCDC_BLINKON);
}

//----------------------------------------------------------------------------------------
//
//		WYďż˝ďż˝cza miganie kursora na LCD
//
//----------------------------------------------------------------------------------------
void lcd_blink_off(void)
{
	lcd_write_cmd( LCDC_ONOFF|LCDC_DISPLAYON);
}
#endif




//----------------------------------------------------------------------------------------
//
//		 ******* INICJALIZACJA WYďż˝WIETLACZA LCD ********
//
//----------------------------------------------------------------------------------------
void lcd_init(void)
{
	// inicjowanie pinďż˝w portďż˝w ustalonych do podďż˝ďż˝czenia z wyďż˝wietlaczem LCD
	// ustawienie wszystkich jako wyjďż˝cia
	data_dir_out();
	DDR(LCD_RSPORT) |= (1<<LCD_RS);
	DDR(LCD_EPORT) |= (1<<LCD_E);
	#if USE_RW == 1
		DDR(LCD_RWPORT) |= (1<<LCD_RW);
	#endif

	PORT(LCD_RSPORT) |= (1<<LCD_RS);
	PORT(LCD_EPORT) |= (1<<LCD_E);
	#if USE_RW == 1
		PORT(LCD_RWPORT) |= (1<<LCD_RW);
	#endif

	_delay_ms(35);
	PORT(LCD_EPORT) &= ~(1<<LCD_E);
	PORT(LCD_RSPORT) &= ~(1<<LCD_RS);
	#if USE_RW == 1	
    	PORT(LCD_RWPORT) &= ~(1<<LCD_RW);
	#endif

	// jeszcze nie moďż˝na uďż˝ywaďż˝ Busy Flag
	SET_E;
	lcd_sendHalf(0x03);	// tryb 8-bitowy
	CLR_E;
	_delay_ms(4.1);

	SET_E;
	lcd_sendHalf(0x03);	// tryb 8-bitowy
	CLR_E;
	_delay_us(100);

	SET_E;
	lcd_sendHalf(0x03);	// tryb 8-bitowy
	CLR_E;
	_delay_us(100);

	SET_E;
	lcd_sendHalf(0x02);// tryb 4-bitowy
	CLR_E;
	_delay_us(100);

	// juďż˝ moďż˝na uďż˝ywaďż˝ Busy Flag
	// tryb 4-bitowy, 2 wiersze, znak 5x7
	lcd_write_cmd( LCDC_FUNC|LCDC_FUNC4B|LCDC_FUNC2L|LCDC_FUNC5x7 );
	// wyďż˝ďż˝czenie kursora
	lcd_write_cmd( LCDC_ONOFF|LCDC_CURSOROFF );
	// wďż˝ďż˝czenie wyďż˝wietlacza
	lcd_write_cmd( LCDC_ONOFF|LCDC_DISPLAYON );
	// przesuwanie kursora w prawo bez przesuwania zawartoďż˝ci ekranu
	lcd_write_cmd( LCDC_ENTRY|LCDC_ENTRYR );

	// kasowanie ekranu
	lcd_cls();
}
