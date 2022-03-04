/************************************************************************
* 5 semestre - Eng. da Computao - Insper
* Rafael Corsi - rafael.corsi@insper.edu.br
*
* Material:
*  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
*
* Objetivo:
*  - Demonstrar configuraçao do PIO
*
* Periféricos:
*  - PIO
*  - PMC
*
* Log:
*  - 10/2018: Criação
************************************************************************/

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LED 1
#define LED1_PIO PIOA
#define LED1_PIO_ID ID_PIOA
#define LED1_PIO_IDX 0
#define LED1_PIO_ID_MASK (1 << LED1_PIO_IDX)// LED 2
#define LED2_PIO PIOC
#define LED2_PIO_ID ID_PIOC
#define LED2_PIO_IDX 30
#define LED2_PIO_ID_MASK (1 << LED2_PIO_IDX)

// LED 3
#define LED3_PIO PIOB
#define LED3_PIO_ID ID_PIOB
#define LED3_PIO_IDX 2
#define LED3_PIO_ID_MASK (1 << LED3_PIO_IDX)

// Botão 1
#define BUT1_PIO PIOD
#define BUT1_PIO_ID ID_PIOD
#define BUT1_PIO_IDX 28
#define BUT1_PIO_IDX_MASK (1 << BUT1_PIO_IDX)

// Botão 2
#define BUT2_PIO PIOC
#define BUT2_PIO_ID ID_PIOC
#define BUT2_PIO_IDX 31
#define BUT2_PIO_IDX_MASK (1u << BUT2_PIO_IDX)

// Botão 3
#define BUT3_PIO PIOA
#define BUT3_PIO_ID ID_PIOA
#define BUT3_PIO_IDX 19
#define BUT3_PIO_IDX_MASK (1u << BUT3_PIO_IDX)

/************************************************************************/

/************************************************************************/
/* constants                                                            */
/************************************************************************/

#define TIME_LEDS_DEFAULT 50   
#define TIME_LEDS_PRESSED 500  
#define TIMES_FLASHING 5     

/* prototypes                                                           */
/************************************************************************/

void init(void);
void piscar(Pio* pio, uint32_t ul_mask, int interval_time);

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/

/************************************************************************/
/* funções                                                              */
/************************************************************************/

void piscar(Pio* pio, uint32_t ul_mask, int interval_time) {
	pio_clear(pio, ul_mask);
	delay_ms(interval_time);
	pio_set(pio, ul_mask);
	delay_ms(interval_time);
}// Função de inicialização do uC
void init(void) {
	// Inicializa o board clock
	sysclk_init();
	// Desativa WatchDog Timer 
	WDT -> WDT_MR = WDT_MR_WDDIS;
	
	// Ativa os PIOs
	pmc_enable_periph_clk(LED1_PIO_ID);
	pmc_enable_periph_clk(LED2_PIO_ID);
	pmc_enable_periph_clk(LED3_PIO_ID);
	pmc_enable_periph_clk(BUT1_PIO_ID);
	
	// pino ligado ao botao como entrada = pull-up
	pio_set_input(BUT1_PIO, BUT1_PIO_IDX_MASK, PIO_DEFAULT);
	pio_set_input(BUT2_PIO, BUT2_PIO_IDX_MASK, PIO_DEFAULT);
	pio_set_input(BUT3_PIO, BUT3_PIO_IDX_MASK, PIO_DEFAULT);
	
	// ativa pull-up
	pio_pull_up(BUT1_PIO, BUT1_PIO_IDX_MASK, 1);
	pio_pull_up(BUT2_PIO, BUT2_PIO_IDX_MASK, 1);
	pio_pull_up(BUT3_PIO, BUT3_PIO_IDX_MASK, 1);
	
	// Inicializa PIOs
	pio_set_output(LED1_PIO, LED1_PIO_ID_MASK, 0, 0, 0);
	pio_set_output(LED2_PIO, LED2_PIO_ID_MASK, 0, 0, 0);
	pio_set_output(LED3_PIO, LED3_PIO_ID_MASK, 0, 0, 0);
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/
/* Funcao principal chamada na inicalizacao do uC.                      */
int main(void) {
	init();
	
	// super loop
	// aplicacoes embarcadas não devem sair do while(1).
	while (1) {
		piscar(LED1_PIO, LED1_PIO_ID_MASK, TIME_LEDS_DEFAULT);
		piscar(LED2_PIO, LED2_PIO_ID_MASK, TIME_LEDS_DEFAULT);
		piscar(LED3_PIO, LED3_PIO_ID_MASK, TIME_LEDS_DEFAULT);
		
		if (!pio_get(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK)) {
			for (int i = 0; i < TIMES_FLASHING; i++) {
				piscar(LED1_PIO, LED1_PIO_ID_MASK, TIME_LEDS_PRESSED);
			}
		}
		
		if (!pio_get(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK)) {
			for (int i = 0; i < TIMES_FLASHING; i++) {
				piscar(LED2_PIO, LED2_PIO_ID_MASK, TIME_LEDS_PRESSED);
			}
		}
		 
		 if (!pio_get(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK)) {
			 for (int i = 0; i < TIMES_FLASHING; i++) {
				 piscar(LED3_PIO, LED3_PIO_ID_MASK, TIME_LEDS_PRESSED);
			}
		}
}
return 0;
}