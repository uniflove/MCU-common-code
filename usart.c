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

#endif //_____region_____ // includes //////////////////////////

#define USART_SERIAL                     &USARTD0
#define USART_SERIAL_BAUDRATE            115200
#define USART_SERIAL_CHAR_LENGTH         USART_CHSIZE_8BIT_gc
#define USART_SERIAL_PARITY              USART_PMODE_DISABLED_gc
#define USART_SERIAL_STOP_BIT            false

#define USART_BUFFER_SIZE			64		// use buffer size one of 32, 64, 128, 256
#define USART_BUFFER_MASK			0x3f	// define buffer counter mask depends on the size

struct _usart_fifo {
	uint8_t txBuffer[USART_BUFFER_SIZE];
	uint8_t rxBuffer[USART_BUFFER_SIZE];
	uint8_t txReadPos;
	uint8_t txWritePos;
	uint8_t rxReadPos;
	uint8_t rxWritePos;
	} usart_t;


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
	
	//sysclk_enable_module((SYSCLK_PORT_D, PR_USART0_bm));
	usart_init_rs232(USART_SERIAL, & usartOptions);
	usart_set_rx_interrupt_level(USART_SERIAL, USART_INT_LVL_LO);
	
	usart_put_string("This is by interrupt...");
	uint8_t temp = 10;
	char buf[32];
	sprintf(buf, "Value is : %d\r\n", temp);
	usart_put_string(buf);
	
	while(1);
}

void usart_put_string(const char *str) {
	uint8_t strLength = strlen(str);
	
	for( ; strLength > 0; strLength--) {
		usart_t.txBuffer[usart_t.txWritePos++] = *str++;
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
	usart_t.rxBuffer[usart_t.rxWritePos++] = usart_getchar(USART_SERIAL);
}