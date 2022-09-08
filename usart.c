#ifndef  _____region_____ // File Header //////////////////////////

/**
* \file
*
* \brief Empty user application template
*
*/

/**
* \mainpage User Application template doxygen documentation
*
* \par Empty user application template
*
* Bare minimum empty user application template
*
* \par Content
*
* -# Include the ASF header files (through asf.h)
* -# "Insert system clock initialization code here" comment
* -# Minimal main function that starts with a call to board_init()
* -# "Insert application code here" comment
*
*/

/*
* Include header files for all drivers that have been imported from
* Atmel Software Framework (ASF).
*/
/*
* Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
*/

#endif //_____region_____ // File Header //////////////////////////

#ifndef  _____region_____ // includes //////////////////////////

#include <asf.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#endif //_____region_____ // includes //////////////////////////

#define USART_SERIAL                     &USARTD0
#define USART_SERIAL_BAUDRATE            115200
#define USART_SERIAL_CHAR_LENGTH         USART_CHSIZE_8BIT_gc
#define USART_SERIAL_PARITY              USART_PMODE_DISABLED_gc
#define USART_SERIAL_STOP_BIT            false

#define USART_BUFFER_SIZE			64		// use buffer size one of 32, 64, 128, 256
#define USART_BUFFER_MASK			0x3f	// define buffer counter mask depends on the size

#define USART_RX_CHAR_ENTER			0x0d

#define USART_CLI_CMD_NUMB			2

struct _usart_fifo {
	uint8_t txBuffer[USART_BUFFER_SIZE];
	uint8_t rxBuffer[USART_BUFFER_SIZE];
	uint8_t txReadPos;
	uint8_t txWritePos;
	uint8_t rxReadPos;
	uint8_t rxWritePos;
	uint8_t rxEnterPos;
} usart_t;

// callback functions for CLI commands
void abcFunc(void);
void bbcFunc(void);

struct _cmd_list {
	char cmd[10];
	void(*cmdFunction)(void);
};

// CLI command list
struct _cmd_list cmdList[USART_CLI_CMD_NUMB] = {
	{"abc", abcFunc}, {"bbc", bbcFunc}
};

void usart_put_string(const char *str);


int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	sysclk_init();
	board_init();

	irq_initialize_vectors();
	cpu_irq_enable();

	/* Insert application code here, after the board has been initialized. */
	usart_rs232_options_t usartOptions =  {
		.baudrate = USART_SERIAL_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT
	};
	
	usart_init_rs232(USART_SERIAL, & usartOptions);
	usart_set_rx_interrupt_level(USART_SERIAL, USART_INT_LVL_LO);
	
	usart_put_string("This is by interrupt...");
	uint8_t temp = 10;
	char buf[32];
	sprintf(buf, "Value is : %d\r\n", temp);
	usart_put_string(buf);
	
	while(1) {
		if(usart_t.rxEnterPos) {
			// copy CLI command from rx buffer
			uint16_t cmdLength = USART_BUFFER_SIZE - usart_t.rxReadPos + usart_t.rxEnterPos + 1;
			if(usart_t.rxEnterPos > usart_t.rxReadPos) {
				cmdLength -= USART_BUFFER_SIZE;
			}
			char cmdString[cmdLength];
			for(uint8_t i = 0; i < cmdLength; i++) {
				cmdString[i] = usart_t.rxBuffer[usart_t.rxReadPos++];
				usart_t.rxReadPos &= USART_BUFFER_MASK;
			}
			cmdString[cmdLength -1] = 0;
			
			// compare command and execute
			for(int i = 0; i<USART_CLI_CMD_NUMB; i++) {
				if(strcmp(cmdString, cmdList[i].cmd) == 0) {
					cmdList[i].cmdFunction();
					break;
				}
			}
			usart_t.rxEnterPos = 0;
		}
	}
}

void usart_put_string(const char *str) {
	uint8_t strLength = strlen(str);
	
	for( ; strLength > 0; strLength--) {
		usart_t.txBuffer[usart_t.txWritePos++] = *str++;
		usart_t.txWritePos &= USART_BUFFER_MASK;
	}
	usart_set_dre_interrupt_level(USART_SERIAL, USART_INT_LVL_LO);
}

ISR(USARTD0_DRE_vect) {
	if(usart_t.txReadPos == usart_t.txWritePos) {
		usart_set_dre_interrupt_level(USART_SERIAL, USART_INT_LVL_OFF);
	}
	else {
		usart_putchar(USART_SERIAL, usart_t.txBuffer[usart_t.txReadPos++]);
		usart_t.txReadPos &= USART_BUFFER_MASK;
	}
}

ISR(USARTD0_RXC_vect) {
	uint8_t rxChar = usart_getchar(USART_SERIAL);
	if(rxChar == USART_RX_CHAR_ENTER) {
		usart_t.rxEnterPos = usart_t.rxWritePos;
	}
	usart_t.rxBuffer[usart_t.rxWritePos++] = rxChar;
	usart_t.rxWritePos &= USART_BUFFER_MASK;
}

void abcFunc(void) {
	usart_put_string("CLI test ok...\r\n");
}

void bbcFunc(void ) {
	usart_put_string("bbc is good for English...\r\n");
}