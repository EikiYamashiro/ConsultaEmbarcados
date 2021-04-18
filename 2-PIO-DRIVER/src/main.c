/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define LED_PIO           PIOC                 // periferico que controla o LED
#define LED_PIO_ID        ID_PIOC                   // ID do periférico PIOC (controla LED)
#define LED_PIO_IDX       8                    // ID do LED no PIO
#define LED_PIO_IDX_MASK  (1 << LED_PIO_IDX)   // Mascara para CONTROLARMOS o LED

// Configuracoes do botao
#define BUT_PIO			  PIOA
#define BUT_PIO_ID		  ID_PIOA
#define BUT_PIO_IDX		  11
#define BUT_PIO_IDX_MASK  (1u << BUT_PIO_IDX)

// Brincando com o oled1 
// LED1
#define LED1_PIO           PIOA                 
#define LED1_PIO_ID        ID_PIOA               
#define LED1_PIO_IDX       0                  
#define LED1_PIO_IDX_MASK  (1 << LED1_PIO_IDX)  

// LED2
#define LED2_PIO           PIOC
#define LED2_PIO_ID        ID_PIOC
#define LED2_PIO_IDX       30
#define LED2_PIO_IDX_MASK  (1 << LED2_PIO_IDX)

// LED3
#define LED3_PIO           PIOB
#define LED3_PIO_ID        ID_PIOB
#define LED3_PIO_IDX       2
#define LED3_PIO_IDX_MASK  (1 << LED3_PIO_IDX)

// BT1
#define BUT1_PIO			PIOD
#define BUT1_PIO_ID		    ID_PIOD
#define BUT1_PIO_IDX		28
#define BUT1_PIO_IDX_MASK   (1u << BUT1_PIO_IDX)

// BT2
#define BUT2_PIO			PIOC
#define BUT2_PIO_ID		    ID_PIOC
#define BUT2_PIO_IDX		31
#define BUT2_PIO_IDX_MASK   (1u << BUT2_PIO_IDX)

// BT3
#define BUT3_PIO			PIOA
#define BUT3_PIO_ID		    ID_PIOA
#define BUT3_PIO_IDX		19
#define BUT3_PIO_IDX_MASK   (1u << BUT3_PIO_IDX)

/*  Default pin configuration (no attribute). */
#define _PIO_DEFAULT             (0u << 0)
/*  The internal pin pull-up is active. */
#define _PIO_PULLUP              (1u << 0)
/*  The internal glitch filter is active. */
#define _PIO_DEGLITCH            (1u << 1)
/*  The internal debouncing filter is active. */
#define _PIO_DEBOUNCE            (1u << 3)

#define cpu_ms_2_cy(ms, f_cpu)  \
(((uint64_t)(ms) * (f_cpu) + (uint64_t)(5.932e3 - 1ul)) / (uint64_t)5.932e3)

/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

/**
 * \brief Set a high output level on all the PIOs defined in ul_mask.
 * This has no immediate effects on PIOs that are not output, but the PIO
 * controller will save the value if they are changed to outputs.
 *
 * \param p_pio Pointer to a PIO instance.
 * \param ul_mask Bitmask of one or more pin(s) to configure.
 */

void _delay(int tempo_ms){
	int clock_value = sysclk_get_cpu_hz();
	delay_cycles(cpu_ms_2_cy(tempo_ms, clock_value));
}

void _pio_set(Pio *p_pio, const uint32_t ul_mask)
{
	p_pio->PIO_SODR = ul_mask;

}


//Seta no register que o PIO recebido no argumento receberá um clear, olhando a posição da mask!
void _pio_clear(Pio *p_pio, const uint32_t ul_mask)
{
	p_pio->PIO_CODR = ul_mask;
}

void _pio_pull_up(Pio *p_pio, const uint32_t ul_mask, const uint32_t ul_pull_up_enable) {
	if (ul_pull_up_enable) {
		p_pio->PIO_PUER = ul_mask;
	} else {
		p_pio->PIO_PUDR = ul_mask;
	}
}

void _pio_set_input(Pio *p_pio, const uint32_t ul_mask,
const uint32_t ul_attribute)
{	
	_pio_pull_up(p_pio, ul_mask, ul_attribute);
	
	if (ul_attribute & PIO_PULLUP) {
		p_pio->PIO_IFER = ul_mask;
	} else {
		p_pio->PIO_IFDR = ul_mask;
	}
	
	
}

void _pio_set_output(Pio *p_pio, const uint32_t ul_mask,
	const uint32_t ul_default_level,
	const uint32_t ul_multidrive_enable,
	const uint32_t ul_pull_up_enable) {
	
	_pio_pull_up(p_pio, ul_mask, ul_pull_up_enable);
	
	if (ul_multidrive_enable) {
		p_pio->PIO_MDER = ul_mask;
	} else {
		p_pio->PIO_MDDR = ul_mask;
	}
	if (ul_default_level) {
		p_pio->PIO_SODR = ul_mask;
	} else {
		p_pio->PIO_CODR = ul_mask;
	}
	p_pio->PIO_OER = ul_mask;
	p_pio->PIO_PER = ul_mask;
}

uint32_t _pio_get(Pio *p_pio, const pio_type_t ul_type,
const uint32_t ul_mask)
{
	uint32_t k;
	// Se for um output, aponta para o OUTPUT DATA STATUS REGISTER!
	if ((ul_type == PIO_OUTPUT_0)) {
		k = p_pio->PIO_ODSR;
		} else {
			// Recebe o input considerando o clock, que é o Pin Data Status Register!!!
		k = p_pio->PIO_PDSR;
	}
	
	// Verifica se o input foi acionado (ligado ao GND) assim retorna 0!
	if ((k & ul_mask) == 0) {
		return 0;
	
	// Se nao for acionado, deve retornar 1!!	
	} else {
		return 1;
	}
}

// Função de inicialização do uC
void init(void)
{
	sysclk_init();
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	pmc_enable_periph_clk(ID_PIOA);
	pmc_enable_periph_clk(ID_PIOB);
	pmc_enable_periph_clk(ID_PIOC);
	pmc_enable_periph_clk(ID_PIOD);
	
	_pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 0, 0, 0);
	
	_pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);
	
	// oled1
	_pio_set_output(LED1_PIO, LED1_PIO_IDX_MASK, 0, 0, 0);
	_pio_set_output(LED2_PIO, LED2_PIO_IDX_MASK, 0, 0, 0);
	_pio_set_output(LED3_PIO, LED3_PIO_IDX_MASK, 0, 0, 0);
	
	_pio_set_input(BUT1_PIO, BUT1_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(BUT2_PIO, BUT2_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);
	_pio_set_input(BUT3_PIO, BUT3_PIO_IDX_MASK, _PIO_PULLUP | _PIO_DEBOUNCE);
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/


int main(void)
{
  init();
	sysclk_init();
	delay_init();
	
  while (1){
	_pio_set(PIOC, LED_PIO_IDX_MASK);
	_pio_set(PIOA, LED1_PIO_IDX_MASK);
	_pio_set(PIOC, LED2_PIO_IDX_MASK);
	_pio_set(PIOB, LED3_PIO_IDX_MASK);
	if (_pio_get(PIOA, PIO_INPUT, BUT_PIO_IDX_MASK) == 0){
		for(int i=0; i<5; i++){
			_pio_set(PIOC, LED_PIO_IDX_MASK);      
			_delay(200);                        
			_pio_clear(PIOC, LED_PIO_IDX_MASK);    
			_delay(200);                        
		}
	}
	if (_pio_get(PIOD, PIO_INPUT, BUT1_PIO_IDX_MASK) == 0){
		for(int i=0; i<5; i++){
			_pio_set(PIOA, LED1_PIO_IDX_MASK);     
			delay_ms(200);                        
			_pio_clear(PIOA, LED1_PIO_IDX_MASK);    
			_delay(200);                       
		}	
	}
	if (_pio_get(PIOC, PIO_INPUT, BUT2_PIO_IDX_MASK) == 0){
		for(int i=0; i<5; i++){
			_pio_set(PIOC, LED2_PIO_IDX_MASK);     
			_delay(200);                        
			_pio_clear(PIOC, LED2_PIO_IDX_MASK);    
			_delay(200);                        
		}
	}
	if (_pio_get(PIOA, PIO_INPUT, BUT3_PIO_IDX_MASK) == 0){
		for(int i=0; i<5; i++){
			_pio_set(PIOB, LED3_PIO_IDX_MASK);      
			_delay(200);                        
			_pio_clear(PIOB, LED3_PIO_IDX_MASK);    
			_delay(200);                        
		}
	}
		
	
	  
	  
	  
	  
  }
  return 0;
}
