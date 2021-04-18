#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"
#include <string.h>

// Botao1 OLED
#define BUT1_PIO			PIOD
#define BUT1_PIO_ID		    ID_PIOD
#define BUT1_PIO_IDX		28
#define BUT1_PIO_IDX_MASK   (1u << BUT1_PIO_IDX)

// LED1
#define LED1_PIO           PIOA
#define LED1_PIO_ID        ID_PIOA
#define LED1_PIO_IDX       0
#define LED1_PIO_IDX_MASK  (1 << LED1_PIO_IDX)

// Botao2 OLED
#define BUT2_PIO			PIOC
#define BUT2_PIO_ID		    ID_PIOC
#define BUT2_PIO_IDX		31
#define BUT2_PIO_IDX_MASK   (1u << BUT2_PIO_IDX)

// LED2
#define LED2_PIO           PIOC
#define LED2_PIO_ID        ID_PIOC
#define LED2_PIO_IDX       30
#define LED2_PIO_IDX_MASK  (1 << LED2_PIO_IDX)

// Botao3 OLED
#define BUT3_PIO			PIOA
#define BUT3_PIO_ID		    ID_PIOA
#define BUT3_PIO_IDX		19
#define BUT3_PIO_IDX_MASK   (1u << BUT3_PIO_IDX)

// LED3
#define LED3_PIO           PIOB
#define LED3_PIO_ID        ID_PIOB
#define LED3_PIO_IDX       2
#define LED3_PIO_IDX_MASK  (1 << LED3_PIO_IDX)

// LED da Placa
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

// Botão da Placa
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX  11
#define BUT_IDX_MASK (1 << BUT_IDX)

// Variaveis Globais
volatile char but_flag = 0;
volatile char but1_flag = 0;
volatile char but2_flag = 0;
volatile char but3_flag = 0;
volatile int freq = 3;

// Prototype
void io_init(void);
void pisca_led(int n, int t);

// Handler / Callbacks

void but_callback(void) {
	but_flag = 1;
}

void but1_callback(void) {
	but1_flag = 1;
}

void but2_callback(void) {
	but2_flag = !but2_flag;
}

void but3_callback(void) {
	but3_flag = 1;
}

void aumenta_frequencia(void) {
	if(freq <= 10){
		freq += 1;
	}
};

void diminui_frequencia(void) {
	if(freq > 1){
		freq -= 1;
	}
};

void pisca(int n) {
	but_flag = 0;
	char str1[14];
	char str2[14];
	int time_ = 30000;
	int delay = 1000000/freq;
	int max_i = (1000*time_/delay);
	for (int i=0; i<max_i; i++) {
		
		if (but1_flag) {
			aumenta_frequencia();
			gfx_mono_draw_string("+", 90,0, &sysfont);
			delay_ms(500);
			gfx_mono_draw_string(" ", 90,0, &sysfont);
			but1_flag = 0;
		}
		
		if (but3_flag) {
			diminui_frequencia();
			gfx_mono_draw_string("-", 20,0, &sysfont);
			delay_ms(500);
			gfx_mono_draw_string(" ", 20,0, &sysfont);
			but3_flag = 0;
		}
		
		delay = 1000000/freq;
		max_i = (1000*time_/delay);
		
		while(but2_flag==1){
			pio_clear(PIOA, LED1_PIO_IDX_MASK);
			pio_clear(PIOC, LED2_PIO_IDX_MASK);
			pio_clear(PIOB, LED3_PIO_IDX_MASK);
			gfx_mono_draw_string("    PAUSE    ", 0,16, &sysfont);
		}
		but2_flag = 0;
		
		float temp = (13*i)/max_i;
		int progress = (int) temp;
		strcpy (str1," ");
		strcpy (str2,"ooooooooooooo");
		strncat (str1, str2, progress);
		gfx_mono_draw_string("             ", 0,16, &sysfont);
		gfx_mono_draw_string(str1, 0,16, &sysfont);
		
		char snum[5];
		itoa(freq, snum, 10);
		gfx_mono_draw_string("Hz", 60,0, &sysfont);
		gfx_mono_draw_string(snum, 40,0, &sysfont);
		
		
		pio_clear(LED_PIO, LED_IDX_MASK);
		delay_us(delay/2);
		pio_set(LED_PIO, LED_IDX_MASK);
		delay_us(delay/2);
	}
	gfx_mono_draw_string("             ", 0,16, &sysfont);
	gfx_mono_draw_string("    LAB 3    ", 0,16, &sysfont);
	
};

void io_init(void) {
	
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(LED1_PIO_ID);
	pmc_enable_periph_clk(LED2_PIO_ID);
	pmc_enable_periph_clk(LED3_PIO_ID);
	
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEBOUNCE);
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_PIO_IDX_MASK, PIO_DEBOUNCE);
	pio_configure(LED2_PIO, PIO_OUTPUT_0, LED2_PIO_IDX_MASK, PIO_DEBOUNCE);
	pio_configure(LED3_PIO, PIO_OUTPUT_0, LED3_PIO_IDX_MASK, PIO_DEBOUNCE);
	
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);
	
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_PIO_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_PIO_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_PIO_IDX_MASK, PIO_PULLUP);
	
	pio_handler_set(BUT_PIO, BUT_PIO_ID, BUT_IDX_MASK, PIO_IT_FALL_EDGE, but_callback);
	pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but1_callback);
	pio_handler_set(BUT2_PIO, BUT2_PIO_ID, BUT2_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but2_callback);
	pio_handler_set(BUT3_PIO, BUT3_PIO_ID, BUT3_PIO_IDX_MASK, PIO_IT_FALL_EDGE, but3_callback);

	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);
	pio_enable_interrupt(BUT1_PIO, BUT1_PIO_IDX_MASK);
	pio_enable_interrupt(BUT2_PIO, BUT2_PIO_IDX_MASK);
	pio_enable_interrupt(BUT3_PIO, BUT3_PIO_IDX_MASK);
	
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_EnableIRQ(BUT3_PIO_ID);
	
	NVIC_SetPriority(BUT_PIO_ID, 4);
	NVIC_SetPriority(BUT1_PIO_ID, 2);
	NVIC_SetPriority(BUT2_PIO_ID, 2);
	NVIC_SetPriority(BUT3_PIO_ID, 2);
}

int main (void)
{
	board_init();
	sysclk_init();
	delay_init();
	gfx_mono_ssd1306_init();
	gfx_mono_draw_string("LAB 3", 50,16, &sysfont);
	io_init();
	
	while(1) {
		
		if (but_flag) {
			pisca(26);
			but_flag = 0;
		}
				
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
	
}
