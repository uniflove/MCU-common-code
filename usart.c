///////////////////////////////////////////////////

/*
Xmega32A4U
Atmel Studio
USART TX code
	Chip dependency 
		ISR Vectors
		USART init
	IDE dependency
		uint8_t
other codes are all the same for any MCU
*/
#include <asf.h>
#include <string.h>
#include <stdio.h>

#define USART_SERIAL                     &USARTD0
#define USART_SERIAL_BAUDRATE            115200
#define USART_SERIAL_CHAR_LENGTH         USART_CHSIZE_8BIT_gc
#define USART_SERIAL_PARITY              USART_PMODE_DISABLED_gc
#define USART_SERIAL_STOP_BIT            false

#ifndef	___region___my_defines //fdhsakrgj;lgjae;wlgkjaw;lkegja;wlekg

#define USART_BUFFER_SIZE			64
#define USART_BUFFER_MASK			0x3f
uint8_t usartTxBuffer[USART_BUFFER_SIZE];
uint8_t usartRxBuffer[USART_BUFFER_SIZE];
uint8_t usartTxBufReadPos;
uint8_t usartTxBufWritepos;
uint8_t usartRxBufReadPos;
uint8_t usartRxBufWritePos;

#endif

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
		usartTxBuffer[usartTxBufWritepos++] = *str++;
	}
	usart_set_dre_interrupt_level(USART_SERIAL, USART_INT_LVL_LO);
}

ISR(USARTD0_DRE_vect) {
	if(usartTxBufReadPos == usartTxBufWritepos) {
		usart_set_dre_interrupt_level(USART_SERIAL, USART_INT_LVL_OFF);
	}
	else {
		usart_putchar(USART_SERIAL, usartTxBuffer[usartTxBufReadPos++]);
		usartTxBufReadPos &= USART_BUFFER_MASK;
	}
}
