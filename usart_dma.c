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
#include <asf.h>
#include <string.h>
#include <stdio.h>

#define USART_SERIAL                     &USARTD0
#define USART_SERIAL_BAUDRATE            115200
#define USART_SERIAL_CHAR_LENGTH         USART_CHSIZE_8BIT_gc
#define USART_SERIAL_PARITY              USART_PMODE_DISABLED_gc
#define USART_SERIAL_STOP_BIT            false

#define DMA_TX_CHANNEL						0
#define DMA_RX_CHANNEL						1

#define USART_BUFFER_SIZE		64
#define USART_BUFFER_MASK		0x3f

#define USART_CLI_CMD_NUMB			2

uint8_t txBuf[USART_BUFFER_SIZE];
uint8_t rxBuf[USART_BUFFER_SIZE];
uint8_t rxCheckPos;
uint8_t rxEnterPos;
uint8_t rxReadPos;

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

void dma_transfer_done(enum dma_channel_status status) {
	usart_putchar(USART_SERIAL, '\r');
	usart_putchar(USART_SERIAL, '\n');
	usart_putchar(USART_SERIAL, 'd');
	usart_putchar(USART_SERIAL, 'o');
	usart_putchar(USART_SERIAL, 'n');
	usart_putchar(USART_SERIAL, 'e');
	usart_putchar(USART_SERIAL, '.');
}

void dma_rx_transfer_done(enum dma_channel_status status) {
	dma_channel_enable(DMA_RX_CHANNEL);
}

static void usart_tx_dma_init(uint8_t *buffer, uint16_t size)
{
	memcpy(txBuf, buffer, size);
	struct dma_channel_config dmach_conf;
	memset(&dmach_conf, 0, sizeof(dmach_conf));
	dma_channel_set_burst_length(&dmach_conf, DMA_CH_BURSTLEN_1BYTE_gc);
	dma_channel_set_transfer_count(&dmach_conf, size);
	dma_channel_set_src_reload_mode(&dmach_conf,
	DMA_CH_SRCRELOAD_NONE_gc);
	dma_channel_set_dest_reload_mode(&dmach_conf,
	DMA_CH_DESTRELOAD_NONE_gc);
	dma_channel_set_src_dir_mode(&dmach_conf, DMA_CH_SRCDIR_INC_gc);
	dma_channel_set_dest_dir_mode(&dmach_conf, DMA_CH_DESTDIR_FIXED_gc);
	dma_channel_set_source_address(&dmach_conf,
	(uint16_t)(uintptr_t)txBuf);
	dma_channel_set_destination_address(&dmach_conf,
	(uint16_t)(uintptr_t)&USARTD0.DATA);
	dma_channel_set_trigger_source(&dmach_conf, DMA_CH_TRIGSRC_USARTD0_DRE_gc);
	dma_channel_set_single_shot(&dmach_conf);
	
	//dma_enable();
	dma_set_callback(DMA_TX_CHANNEL, dma_transfer_done);
	dma_channel_set_interrupt_level(&dmach_conf, DMA_INT_LVL_LO);
	dma_channel_write_config(DMA_TX_CHANNEL, &dmach_conf);
	dma_channel_enable(DMA_TX_CHANNEL);
}

static void usart_rx_dma_init(uint8_t *buffer, uint16_t size)
{
	struct dma_channel_config dmach_conf;
	memset(&dmach_conf, 0, sizeof(dmach_conf));
	dma_channel_set_burst_length(&dmach_conf, DMA_CH_BURSTLEN_1BYTE_gc);
	dma_channel_set_transfer_count(&dmach_conf, size);
	dma_channel_set_src_reload_mode(&dmach_conf,
	DMA_CH_SRCRELOAD_NONE_gc);
	dma_channel_set_dest_reload_mode(&dmach_conf,
	DMA_CH_DESTRELOAD_BLOCK_gc);
	dma_channel_set_src_dir_mode(&dmach_conf, DMA_CH_SRCDIR_FIXED_gc);
	dma_channel_set_dest_dir_mode(&dmach_conf, DMA_CH_DESTDIR_INC_gc);
	dma_channel_set_source_address(&dmach_conf,
	(uint16_t)(uintptr_t)&USARTD0.DATA);
	dma_channel_set_destination_address(&dmach_conf,
	(uint16_t)(uintptr_t)buffer);
	dma_channel_set_trigger_source(&dmach_conf, DMA_CH_TRIGSRC_USARTD0_RXC_gc);
	dma_channel_set_single_shot(&dmach_conf);
	
	//dma_enable();
	dma_set_callback(DMA_RX_CHANNEL, dma_rx_transfer_done);
	dma_channel_set_interrupt_level(&dmach_conf, DMA_INT_LVL_LO);
	dma_channel_write_config(DMA_RX_CHANNEL, &dmach_conf);
	dma_channel_enable(DMA_RX_CHANNEL);
}

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
	
	dma_enable();
	
	uint8_t buf[32] = "This is from DMA transfer..";
	usart_tx_dma_init(buf, strlen(buf));
	usart_rx_dma_init(rxBuf, sizeof(rxBuf));
	
	while(1) {
		uint8_t rxWritePos = USART_BUFFER_SIZE - dma_get_channel_address_from_num(1)->TRFCNTL;
		if(rxCheckPos != rxWritePos) {
			if(rxBuf[rxCheckPos++] == 0x0d) {
				rxEnterPos = rxCheckPos - 1;
				uint16_t cmdLength = USART_BUFFER_SIZE - rxReadPos + rxEnterPos + 1;
				if(rxEnterPos > rxReadPos) {
					cmdLength -= USART_BUFFER_SIZE;
				}
				char cmdString[cmdLength];
				for(uint8_t i = 0; i < cmdLength; i++) {
					cmdString[i] = rxBuf[rxReadPos++];
					rxReadPos &= USART_BUFFER_MASK;
				}
				cmdString[cmdLength -1] = 0;
		
				// compare command and execute
				for(int i = 0; i<USART_CLI_CMD_NUMB; i++) {
					if(strcmp(cmdString, cmdList[i].cmd) == 0) {
						cmdList[i].cmdFunction();
						break;
					}
				}
				rxEnterPos = 0;				
			}
		}
	}
}

void abcFunc(void) {
	uint8_t buf[32] = "CLI test ok...\r\n";
	usart_tx_dma_init(buf, strlen(buf));
}

void bbcFunc(void ) {
	uint8_t buf[32] = "bbc is good for English...\r\n";
	usart_tx_dma_init(buf, strlen(buf));
}