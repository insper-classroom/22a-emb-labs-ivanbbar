#include <asf.h>
#include <time.h>
#include <string.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/* Botão 1 */
#define BUT1_PIO				PIOD
#define BUT1_PIO_ID				ID_PIOD
#define BUT1_PIO_IDX			28
#define BUT1_PIO_IDX_MASK		(1u << BUT1_PIO_IDX)
#define BUT1_PRIORITY			4

/* TRIG - Pino Y */
#define TRIG_PIO				PIOC
#define TRIG_PIO_ID				ID_PIOC
#define TRIG_PIO_IDX			13
#define TRIG_PIO_IDX_MASK	    (1 << TRIG_PIO_IDX)

/* ECHO - Pino X */
#define ECHO_PIO				PIOA
#define ECHO_PIO_ID				ID_PIOA
#define ECHO_PIO_IDX			4
#define ECHO_PIO_IDX_MASK		(1 << ECHO_PIO_IDX)
#define ECHO_PRIORITY			4

volatile char echo_flag;
volatile char but1_flag = 0;

volatile float freq = (float) 1/(0.000058*2);
volatile float tiempo = 0;

void echo_callback(void);
void but1_callback(void);
void io_init(void);
void display_oled(freq);
static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT));
		rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	
}

void but1_callback(void){
	but1_flag = 1;
}

void echo_callback(void) {
	if (echo_flag) {
		tiempo = rtt_read_timer_value(RTT);
	}
	else {
		RTT_init(freq, 0, 0);
	}
	echo_flag = !echo_flag;
}

void display_oled(int t) {
	char distance_string[20];
	float distance = (float) (340*t*100.0)/(2.0*freq);
	sprintf(distance_string, "%2.2f cm", distance);
	gfx_mono_draw_string(distance_string, 0, 0, &sysfont);
}

void io_init(void) {
	
	board_init();
	
	sysclk_init();

	WDT->WDT_MR = WDT_MR_WDDIS;
	
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(TRIG_PIO);
	pmc_enable_periph_clk(ECHO_PIO);
	
	// configura input e output
	pio_set_input(ECHO_PIO,ECHO_PIO_IDX_MASK,PIO_DEFAULT);
	pio_configure(TRIG_PIO, PIO_OUTPUT_0,TRIG_PIO_IDX_MASK, PIO_DEFAULT);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_DEBOUNCE | PIO_PULLUP);
	pio_set_debounce_filter(BUT1_PIO, BUT1_PIO_IDX_MASK, 60);
	
	// configura interrupções
	pio_handler_set(ECHO_PIO, ECHO_PIO_ID, ECHO_PIO_IDX_MASK, PIO_IT_EDGE, echo_callback);
	pio_enable_interrupt(ECHO_PIO, ECHO_PIO_IDX_MASK);
	pio_get_interrupt_status(ECHO_PIO);
	NVIC_EnableIRQ(ECHO_PIO_ID);
	NVIC_SetPriority(ECHO_PIO_ID, ECHO_PRIORITY);
	
	pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but1_callback);
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, BUT1_PRIORITY);
}

int main (void)
{
	io_init();

	delay_init();

	gfx_mono_ssd1306_init();
	gfx_mono_draw_string("NO DETECT", 0,16, &sysfont);
	
	while(1) {
		if (but1_flag) {
			but1_flag = 0;
			pio_set(TRIG_PIO, TRIG_PIO_IDX_MASK);
			delay_us(10);
			pio_clear(TRIG_PIO, TRIG_PIO_IDX_MASK);
			gfx_mono_draw_filled_rect(0, 0, 128, 32, GFX_PIXEL_CLR);
			display_oled(tiempo);
			}
			
			pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}