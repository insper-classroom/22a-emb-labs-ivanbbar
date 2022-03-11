#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/* LED da placa principal */
#define LED_PIO           PIOC				
#define LED_PIO_ID        ID_PIOC			 
#define LED_PIO_IDX       8					
#define LED_PIO_IDX_MASK  (1 << LED_PIO_IDX) 

/* LED 1 da placa OLED */
#define LED1_PIO          PIOA
#define LED1_PIO_ID       ID_PIOA
#define LED1_PIO_IDX      0
#define LED1_PIO_ID_MASK  (1 << LED1_PIO_IDX)

/* Bot�o 1 da placa OLED */
#define BUT1_PIO          PIOD
#define BUT1_PIO_ID       ID_PIOD
#define BUT1_PIO_IDX      28
#define BUT1_PIO_IDX_MASK (1 << BUT1_PIO_IDX)

// Bot�o 2
#define BUT2_PIO PIOC
#define BUT2_PIO_ID ID_PIOC
#define BUT2_PIO_IDX 31
#define BUT2_PIO_IDX_MASK (1u << BUT2_PIO_IDX)

// Bot�o 3
#define BUT3_PIO PIOA
#define BUT3_PIO_ID ID_PIOA
#define BUT3_PIO_IDX 19
#define BUT3_PIO_IDX_MASK (1u << BUT3_PIO_IDX)

volatile char but1_flag;
volatile char but2_flag;
volatile char but3_flag;

int freq = 130;  // porque 13 � o numero da sorte
volatile char str[128];

void but1_callback(void);
void but2_callback(void);
void but3_callback(void);

void pisca_led1(int n, int i);
void io_init(void);
void display_oled(freq);
void desliga(void);

void but1_callback(void) {
	but1_flag = 1;
}

void but2_callback(void) {
	but2_flag = 1;
}

void but3_callback(void) {
	but3_flag = 1;
}


void pisca_led1(int n, int t) {
	double dt = (double)n / t;
	int time_pisca = dt * 1000; // delay em ms
	display_oled(freq);
	gfx_mono_draw_horizontal_line(0, 10, 4*(n-1), GFX_PIXEL_SET);
	for (int i = 0; i < n; i++) {
		pio_clear(LED1_PIO, LED1_PIO_ID_MASK);
		delay_ms(time_pisca);
		pio_set(LED1_PIO, LED1_PIO_ID_MASK);
		delay_ms(time_pisca);
		gfx_mono_draw_horizontal_line(0, 10, 4*i, GFX_PIXEL_CLR);
		if (but2_flag) {
			pio_set(LED1_PIO, LED1_PIO_ID_MASK);
			gfx_mono_draw_horizontal_line(0, 10, 4*(n-1), GFX_PIXEL_CLR);
			delay_ms(30);
			break;
		}
	}
}

void desliga(void) {
	pio_set(LED1_PIO, LED1_PIO_ID_MASK);
	delay_ms(30);
}

void display_oled(freq) {
	char freq_string[15];
	sprintf(freq_string, "f=%d Hz", freq);
	gfx_mono_draw_string(freq_string, 10, 16, &sysfont);
}

void io_init(void) {
	// Configura o LED
	pmc_enable_periph_clk(LED1_PIO_ID);
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_PIO_ID_MASK, PIO_DEFAULT);

	// Clock do perif�rico PIO que controla o bot�o do OLED
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);
	
	// PIO lida com pino do bot�o como entrada com pull-up e debounce
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_PIO, BUT1_PIO_IDX_MASK, 60);
	pio_set_debounce_filter(BUT2_PIO, BUT2_PIO_IDX_MASK, 60);
	pio_set_debounce_filter(BUT3_PIO, BUT3_PIO_IDX_MASK, 60);
	
	// Configura interrup��o no pinto do bot�o e associa a ele o callback
	pio_handler_set(BUT1_PIO,
	BUT1_PIO_ID,
	BUT1_PIO_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but1_callback);
	
	pio_handler_set(BUT2_PIO,
	BUT2_PIO_ID,
	BUT2_PIO_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but2_callback);
	
	pio_handler_set(BUT3_PIO,
	BUT3_PIO_ID,
	BUT3_PIO_IDX_MASK,
	PIO_IT_FALL_EDGE,
	but3_callback);
	
	// Ativa a interrup��o e limpa a primeira IRQ gerada na ativa��o
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);
	pio_get_interrupt_status(BUT2_PIO);
	pio_get_interrupt_status(BUT3_PIO);
	
	// Configura NVIC para receber interrup��es do PIO do bot�o com prioridade 4
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4);
	NVIC_SetPriority(BUT2_PIO_ID, 4);
	NVIC_SetPriority(BUT3_PIO_ID, 4);
}

int main (void)
{
	board_init();
	
	// Inicia o clock
	sysclk_init();

	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

	// Configura bot�o com interrup��o
	io_init();

	delay_init();

	// Init OLED
	gfx_mono_ssd1306_init();
	
	//gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	//display_oled(freq);

	/* Insert application code here, after the board has been initialized. */
	while(1) {
		if (but1_flag || but2_flag || but3_flag) {
			for (int i = 0; i < 99999999; i++) {
				if (pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK)) {
					if ((!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK) && i >= 9999000) || !pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK)) {
						freq -= 100;
						if (freq <= 0) {
							freq = 130;
						}
						pisca_led1(30, freq);
						break;
					}
					else if ((pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK) && i < 9999000)) {
						freq += 100;
						pisca_led1(30, freq);
						but1_flag = 0;
						break;
					}
				}
				else {
					desliga();
					break;
				}
			}
			but1_flag = 0;
			but2_flag = 0;
			but3_flag = 0;
		}
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}