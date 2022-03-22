#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/* TRIG - Pino Y */
#define TRIG_PIO				PIOC
#define TRIG_PIO_ID				ID_PIOC
#define TRIG_PIO_IDX			13
#define TRIG_PIO_IDX_MASK	    (1u << TRIG_PIO_IDX)

/* ECHO - Pino X */
#define ECHO_PIO				PIOA
#define ECHO_PIO_ID				ID_PIOA
#define ECHO_PIO_IDX			4
#define ECHO_PIO_ID_MASK		(1 << ECHO_PIO_IDX)
#define ECHO_PRIORITY			4

volatile char echo_flag;

int freq = 130;  // porque 13 é o numero da sorte

void echo_callback(void);
void io_init(void);
void display_oled(freq);

void echo_callback(void) {
	echo_flag = 1;
}

void display_oled(freq) {
	char freq_string[15];
	sprintf(freq_string, "f=%d Hz", freq);
	gfx_mono_draw_string(freq_string, 10, 16, &sysfont);
}

void io_init(void) {
	
	board_init();
	
	sysclk_init();

	WDT->WDT_MR = WDT_MR_WDDIS;
	
	pmc_enable_periph_clk(TRIG_PIO);
	pmc_enable_periph_clk(ECHO_PIO);
	
	// configura input e output
	pio_set_input(ECHO_PIO,ECHO_PIO_ID_MASK,PIO_DEFAULT);
	pio_configure(TRIG_PIO, PIO_OUTPUT_0,TRIG_PIO_IDX_MASK, PIO_DEFAULT);
	
	// configura interrupções
	pio_handler_set(ECHO_PIO, ECHO_PIO_ID, ECHO_PIO_ID_MASK, PIO_IT_EDGE, echo_callback);
	pio_enable_interrupt(ECHO_PIO, ECHO_PIO_ID_MASK);
	pio_get_interrupt_status(ECHO_PIO);
	NVIC_EnableIRQ(ECHO_PIO_ID);
	NVIC_SetPriority(ECHO_PIO_ID, ECHO_PRIORITY);
}

int main (void)
{

	io_init();

	delay_init();

	gfx_mono_ssd1306_init();
	
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