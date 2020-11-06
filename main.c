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

/*Physical page
boundaries start at addresses that are
integer multiples of the page buffer size
(or ‘page size’) and end at addresses that
are integer multiples of [page size – 1].

In short, we need o assing the address to a multiple of 8. 
Luckily we do that in both our functions so no worry what you type in.
*/
#define VG_DATA_START_ADDRESS_1 0x10
#define VG_DATA_START_ADDRESS_2 0x18
#define VG_DATA_START_ADDRESS_3 0x20

char wordArray1[9] = "Lasse gi"; // Only 8 characters since we use 8 bytes for eeprom_sequential_read function. 
							    // Last '\0' is discarded inside function. If we surpass 8 bytes the nameArray char will 
								// start overwriting itself. Try it with commenting out first nameArray1 and using the second one below.

//char wordArray1[10] = "LasseHejx"; // This one overwrites itself with 'x' replaceing 'L' turning our char array to xasseHej instead of LasseHejx
char wordArray2[9] = "llar emb";
char wordArray3[9] = "edded :)";

void main (void) {
	uint8_t totalArrayLength = (strlen(wordArray1) + strlen(wordArray2) + strlen(wordArray3));
	i2c_init();
	uart_init();

	sei();

	eeprom_write_page(EEPROM_ADDRESS, VG_DATA_START_ADDRESS_1, wordArray1);
	eeprom_write_page(EEPROM_ADDRESS, VG_DATA_START_ADDRESS_2, wordArray2);
	eeprom_write_page(EEPROM_ADDRESS, VG_DATA_START_ADDRESS_3, wordArray3);

	// Print out the 3 arrays or words we defined up above.
	printf_P(PSTR("String as hexdump: "));
	eeprom_sequential_read(EEPROM_ADDRESS, VG_DATA_START_ADDRESS_1, totalArrayLength, 1); 

	printf_P(PSTR("\nString as regular text: "));
	eeprom_sequential_read(EEPROM_ADDRESS, VG_DATA_START_ADDRESS_1, totalArrayLength, 0); 


	while (1) {

	}
}

