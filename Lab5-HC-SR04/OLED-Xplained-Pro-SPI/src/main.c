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
volatile double distance;

double freq = 340/(2*0.02);

void echo_callback(void);
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

void echo_callback(void) {
	if (echo_flag) {
		volatile double tempo = rtt_read_timer_value(RTT);
	}
	else {
		RTT_init(freq, 0, 0);
	}
	echo_flag = !echo_flag;
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
	gfx_mono_draw_string("NO DETECT", 0,16, &sysfont);
	
	while(1) {
		
	}
}