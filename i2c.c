#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include <stdio.h>
#include <string.h>

#include "i2c.h"

void i2c_init(void) {

    TWSR = 0x00;		// TWSR is the prescaler value. 00 equals prescaler value 1 
    TWBR = 0x48;		// TWBR is the bit rate registry division factor // 0x48 is 72 in decimal 

						// SCLfreq =  F_CPU     / (16+(2 *   TWBR   * Prescaler))
    					// 100 000 = 16 000 000 / (16 + (2 * TWBR   *     1    ))

						// With TWSR and TWBR set to these values we get the Correct SCLfrequency

    TWCR = (1<<TWEN);	// This enables Two Wire Interface (TWI)
}

void i2c_meaningful_status(uint8_t status) {
	switch (status) {
		case 0x08: // START transmitted, proceed to load SLA+W/R
			printf_P(PSTR("START\n"));
			break;
		case 0x10: // repeated START transmitted, proceed to load SLA+W/R
			printf_P(PSTR("RESTART\n"));
			break;
		case 0x38: // NAK or DATA ARBITRATION LOST
			printf_P(PSTR("NOARB/NAK\n"));
			break;
		// MASTER TRANSMIT
		case 0x18: // SLA+W transmitted, ACK received
			printf_P(PSTR("MT SLA+W, ACK\n"));
			break;
		case 0x20: // SLA+W transmitted, NAK received
			printf_P(PSTR("MT SLA+W, NAK\n"));
			break;
		case 0x28: // DATA transmitted, ACK received
			printf_P(PSTR("MT DATA+W, ACK\n"));
			break;
		case 0x30: // DATA transmitted, NAK received
			printf_P(PSTR("MT DATA+W, NAK\n"));
			break;
		// MASTER RECEIVE
		case 0x40: // SLA+R transmitted, ACK received
			printf_P(PSTR("MR SLA+R, ACK\n"));
			break;
		case 0x48: // SLA+R transmitted, NAK received
			printf_P(PSTR("MR SLA+R, NAK\n"));
			break;
		case 0x50: // DATA received, ACK sent
			printf_P(PSTR("MR DATA+R, ACK\n"));
			break;
		case 0x58: // DATA received, NAK sent
			printf_P(PSTR("MR DATA+R, NAK\n"));
			break;
		default:
			printf_P(PSTR("N/A %02X\n"), status);
			break;
	}
}


inline void i2c_start() {
											
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN); // TWINT: Clears the flag
											// TWSTA: Transmits start condition
											// TWEN:  Enables 2-wire serial interface
	while ((TWCR & (1<<TWINT)) == 0); 		// Wait for TWINT Flag set. 
}

inline void i2c_stop() {					// Transmit STOP condition.
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);	// TWSTO: writing bit to one in Master mode will generate a STOP condition on the 2-wire Serial Bus
	while ((TWCR & (1 << TWSTO)));

}

inline uint8_t i2c_get_status(void) {		// Get the status of i2c, see i2c_meaningful_status() for different statuses
	uint8_t status;
    //mask status
    status = TWSR & 0xF8;
    return status;
}

inline void i2c_emit_addr(uint8_t eepromAddress, uint8_t rw) {

	// TWDR is the "original adress" which is 1010 000(0) where the last bit (0) is write "0" or read "1"
	
	// TWDR = (eepromAddress & 0xfe) | (rw & 0x01); This is the same as below. 1010 000(1/0)

	TWDR = (eepromAddress);		   
	TWDR |= (rw << TWD0); 			 	// This one works, but is apparently not recommended???

    TWCR = (1 << TWINT)|(1 << TWEN); 	// Clear TWINT bit in TWCR to start transmission of address
                                      
	while ((TWCR & (1<<TWINT)) == 0);	// Wait for TWINT Flag set. This indicates
										// that the DATA has been transmitted, and
										// ACK/NACK has been received.
}


inline void i2c_emit_byte(uint8_t data) {		// Send one byte at a time
	
	TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN); 	// Clear TWINT bit in TWCR to start transmission of address
    while ((TWCR & (1<<TWINT)) == 0);	// Wait for TWINT Flag set. This indicates
										// that the DATA has been transmitted, and
										// ACK/NACK has been received.

}

inline uint8_t i2c_read_ACK() {
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWEA); // TWEA is the ACK bit here set 1
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}

inline uint8_t i2c_read_NAK() {
    TWCR = (1<<TWINT)|(1<<TWEN);			// TWEA is the ACK bit here NOT set (0), results in NAK
    while ((TWCR & (1<<TWINT)) == 0);
    return TWDR;
}

inline void eeprom_wait_until_write_complete(uint8_t myEepromAdress) {	// Wait for the write to be complete
	while (i2c_get_status() != 0x18) {									// As long as status != 0x18 (SLA+W transmitted, ACK received)
		i2c_start();													// This is acknowledge polling
		i2c_emit_addr(myEepromAdress, I2C_W);
	}
}

uint8_t eeprom_read_byte(uint8_t eepromAddress, uint8_t myDataAddress) { // Read one byte from eeprom
	
	uint8_t returnData;

	i2c_start();							// Start i2c
	i2c_emit_addr(eepromAddress, I2C_W);	// choose the eeprom adress and Write bit
	if(i2c_get_status() != 0x18){			// check if i2c status is 0x18 (SLA+W transmitted, ACK received)
		printf_P(PSTR("Fault 1 Read, current status shown below:\n"));// If something goes wrong we will print
		i2c_meaningful_status(i2c_get_status());						// an error message and exit function
		return 0;
	}

	i2c_emit_byte(myDataAddress);			// Choose where to write byte. Where we want to hangout u know
	if(i2c_get_status() != 0x28){			// check if i2c status is 0x28 DATA transmitted, ACK received
		printf_P(PSTR("Fault 2 Read, current status shown below:\n"));	// If something goes wrong we will print
		i2c_meaningful_status(i2c_get_status());						// an error message and exit function
		return 0;
	}

	i2c_start();							// start i2c again 

	i2c_emit_addr(eepromAddress, I2C_R);	// this time we do read isntead of write bit
	if(i2c_get_status() != 0x40){			// check if i2c status is 0x40 (SLA+R transmitted, ACK received)
		printf_P(PSTR("Fault 3 Read, current status shown below:\n"));	// If something goes wrong we will print
		i2c_meaningful_status(i2c_get_status());						// an error message and exit function
		return 0;
	}
									
	returnData = i2c_read_NAK();			// returnData uint8_t is read_NAK					
	if(i2c_get_status() != 0x58){			// check if i2c status is 0x58  (DATA received, NAK sent)
		printf_P(PSTR("Fault 4 Read, current status shown below:\n"));	// If something goes wrong we will print
		i2c_meaningful_status(i2c_get_status());						// an error message and exit function
		return 0;
	}
	
	i2c_stop();								// stop the i2c 

	return returnData;						// return value received
}

void eeprom_write_byte(uint8_t eepromAdress, uint8_t data, uint8_t myDataAddress) { // Writes one byte to eeprom

	i2c_start();
	i2c_emit_addr(eepromAdress, I2C_W);		// Enter eeprom address and I2c_W(write) or I2C_R(read)
	if(i2c_get_status() != 0x18){			// check if i2c status is 0x18 (SLA+W transmitted, ACK received)
		printf_P(PSTR("Fault 1 Write, current status shown below:\n"));	// If something goes wrong we will print
		i2c_meaningful_status(i2c_get_status());						// an error message and exit function
		return 0;
	}

	i2c_emit_byte(myDataAddress);			// Choose where to write byte. In this case we use parameter myDataAddress
	if(i2c_get_status() != 0x28){			// check if i2c status is 0x28 // DATA transmitted, ACK received
		printf_P(PSTR("Fault 2 Write, current status shown below:\n"));	// If something goes wrong we will print
		i2c_meaningful_status(i2c_get_status());						// an error message and exit function
		return 0;
	}

	i2c_emit_byte(data);					// send in our actual _data_ into myDataAddress										
	if(i2c_get_status() != 0x28){			// check if i2c status is 0x28 (DATA transmitted, ACK received)
		printf_P(PSTR("Fault 3 Write, current status shown below:\n"));	// If something goes wrong we will print
		i2c_meaningful_status(i2c_get_status());						// an error message and exit function
		return 0;
	}

	i2c_stop();								// stop write

}

// Function to write one char at a time to eeproms "myDataAddress++" and then read out from them.
// We take in an array of chars pre chose and write each char[i] from that array into our memory address of choice
// We then read from that address and increment both the char[i] in array and myDataAddress++
// In VG uppgift i will separate this function into read and write.
void i2c_write_name(char * nameArray, uint8_t eepromAdress, uint8_t myDataAddress){
	uint8_t i = 0;
	uint8_t dataToPrint;

	while(nameArray[i] != '\0'){

		eeprom_write_byte(eepromAdress, nameArray[i], myDataAddress);
		eeprom_wait_until_write_complete(eepromAdress);

		printf_P(PSTR("%c"), eeprom_read_byte(eepromAdress, myDataAddress));
		myDataAddress++;
		i++;
	}
}


void eeprom_write_page(uint8_t addr, uint8_t *data) {
	// ... (VG)
}

void eeprom_sequential_read(uint8_t *buf, uint8_t start_addr, uint8_t len) {
	// ... (VG)
}
