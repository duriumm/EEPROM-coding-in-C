#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include "adc.h"
#include "gpio.h"
#include "i2c.h"
#include "serial.h"
#include "timer.h"

#define EEPROM_ADDRESS 0xA0
#define read 1
#define write 0
#define MY_DATA_ADDRESS 60

#define DELUPPGIFT_2_DATA_ADDRESS 0x10


uint8_t eepromData;
char nameArray[6] = "Lasse";

void main (void) {

	i2c_init();
	uart_init();

	sei();
	
	// Test this function to enter one char TO eeprom and then read out the same char FROM eeprom.
	/* 
	eeprom_write_byte(EEPROM_ADDRESS, 'e', MY_DATA_ADDRESS);
	eeprom_wait_until_write_complete(EEPROM_ADDRESS);

	eepromData = eeprom_read_byte(EEPROM_ADDRESS, MY_DATA_ADDRESS);

	printf_P(PSTR("Data from eeprom is: %c\n"),eepromData);
	_delay_ms(1000);
	*/
	

	// Test this function to enter a name (modify it up top (line 22) for your liking) into 0x10++
	// read out the name from address 0x10++. When char is entered we wait for it to finish and the we read it out.
	
	i2c_write_name(nameArray, EEPROM_ADDRESS, DELUPPGIFT_2_DATA_ADDRESS);
	


	while (1) {
					
	}
}

