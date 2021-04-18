#include <asf.h>
#include "conf_board.h"
#include <string.h>
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

#define AFEC_Y				AFEC1
#define AFEC_Y_ID			ID_AFEC1
#define AFEC_Y_CHANNEL		1

#define AFEC_X				AFEC0
#define AFEC_X_ID			ID_AFEC0
#define AFEC_X_CHANNEL		0

/* Botao da placa */
#define BUT_PIO     PIOA
#define BUT_PIO_ID  ID_PIOA
#define BUT_PIO_PIN	11
#define BUT_PIO_PIN_MASK (1 << BUT_PIO_PIN)

/** RTOS  */
#define TASK_OLED_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_OLED_STACK_PRIORITY            (tskIDLE_PRIORITY)

#define TASK_LCD_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_LCD_STACK_PRIORITY            (tskIDLE_PRIORITY)+1

typedef struct {
	char id;
	uint value;
} adcData;

SemaphoreHandle_t xSemaphoreY;
SemaphoreHandle_t xSemaphoreX;

QueueHandle_t xQueueADC;

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

/** prototypes */
void but_callback(void);
static void BUT_init(void);

/************************************************************************/
/* RTOS application funcs                                               */
/************************************************************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) {	}
}

extern void vApplicationIdleHook(void) { }

extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	configASSERT( ( volatile void * ) NULL );
}

/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/

static void AFEC_pot_Callback(void){
	adcData adc;
	adc.id = 'x';
	adc.value = afec_channel_get_value(AFEC_X, AFEC_X_CHANNEL);
	xQueueSendFromISR(xQueueADC, &adc, 0);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreX, &xHigherPriorityTaskWoken);
}

static void AFEC_pot_Callback_Y(void){
	adcData adc;
	adc.id = 'y';
	adc.value = afec_channel_get_value(AFEC_Y, AFEC_Y_CHANNEL);
	xQueueSendFromISR(xQueueADC, &adc, 0);
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(xSemaphoreY, &xHigherPriorityTaskWoken);
}

static void config_AFEC_pot(Afec *afec, uint32_t afec_id, uint32_t afec_channel, afec_callback_t callback){
  /*************************************
  * Ativa e configura AFEC
  *************************************/
  /* Ativa AFEC - 0 */
  afec_enable(afec);

  /* struct de configuracao do AFEC */
  struct afec_config afec_cfg;

  /* Carrega parametros padrao */
  afec_get_config_defaults(&afec_cfg);

  /* Configura AFEC */
  afec_init(afec, &afec_cfg);

  /* Configura trigger por software */
  afec_set_trigger(afec, AFEC_TRIG_SW);

  /*** Configuracao específica do canal AFEC ***/
  struct afec_ch_config afec_ch_cfg;
  afec_ch_get_config_defaults(&afec_ch_cfg);
  afec_ch_cfg.gain = AFEC_GAINVALUE_0;
  afec_ch_set_config(afec, afec_channel, &afec_ch_cfg);

  /*
  * Calibracao:
  * Because the internal ADC offset is 0x200, it should cancel it and shift
  down to 0.
  */
  afec_channel_set_analog_offset(afec, afec_channel, 0x200);

  /***  Configura sensor de temperatura ***/
  struct afec_temp_sensor_config afec_temp_sensor_cfg;

  afec_temp_sensor_get_config_defaults(&afec_temp_sensor_cfg);
  afec_temp_sensor_set_config(afec, &afec_temp_sensor_cfg);
  
  /* configura IRQ */
  afec_set_callback(afec, afec_channel,	callback, 1);
  NVIC_SetPriority(afec_id, 4);
  NVIC_EnableIRQ(afec_id);
}



void but_callback(void) {
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

static void task_oled(void *pvParameters) {
	gfx_mono_ssd1306_init();	
	adcData adc;
	
	for (;;)  {
		if (xQueueReceive( xQueueADC, &(adc), ( TickType_t )  100 / portTICK_PERIOD_MS)) {
			if(adc.id=='x'){
				char b[512];
				int value = adc.value / 400;
				char str1[10];
				char str2[10];
				strcpy (str1," ");
				strcpy (str2,"XXXXXXXXXX");
				strncat (str1, str2, value);
				gfx_mono_draw_string("            ", 0,10, &sysfont);
				gfx_mono_draw_string(str1, 0,10, &sysfont);
			} else {
				char b[512];
				int value = adc.value / 400;
				char str1[10];
				char str2[10];
				strcpy (str1," ");
				strcpy (str2,"YYYYYYYYYY");
				strncat (str1, str2, value);
				gfx_mono_draw_string("            ", 0,0, &sysfont);
				gfx_mono_draw_string(str1, 0,0, &sysfont);
			}
			
			
		}
	}
}

void task_adc_y(void){
	xSemaphoreY = xSemaphoreCreateBinary();
	if (xSemaphoreY == NULL)
	printf("Falha em criar o semaforo \n");

	/* inicializa e configura adc */
	config_AFEC_pot(AFEC_Y, AFEC_Y_ID, AFEC_Y_CHANNEL, AFEC_pot_Callback_Y);

	/* Selecina canal e inicializa conversão */
	afec_channel_enable(AFEC_Y, AFEC_Y_CHANNEL);
	afec_start_software_conversion(AFEC_Y);
	
	while(1){
		const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
		if(xSemaphoreTake(xSemaphoreY, ( TickType_t ) 200 / portTICK_PERIOD_MS) == pdTRUE){
			
			
			vTaskDelay(xDelay);

			/* Selecina canal e inicializa conversão */
			afec_channel_enable(AFEC_Y, AFEC_Y_CHANNEL);
			afec_start_software_conversion(AFEC_Y);
		}
	}
}

void task_adc(void){
	xSemaphoreX = xSemaphoreCreateBinary();
	if (xSemaphoreX == NULL)
	printf("Falha em criar o semaforo \n");

	/* inicializa e configura adc */
	config_AFEC_pot(AFEC_X, AFEC_X_ID, AFEC_X_CHANNEL, AFEC_pot_Callback);

	/* Selecina canal e inicializa conversão */
	afec_channel_enable(AFEC_X, AFEC_X_CHANNEL);
	afec_start_software_conversion(AFEC_X);
	
	while(1){
		const TickType_t xDelay = 10 / portTICK_PERIOD_MS;
		if(xSemaphoreTake(xSemaphoreX, ( TickType_t ) 200 / portTICK_PERIOD_MS) == pdTRUE){
			
			
			vTaskDelay(xDelay);

			/* Selecina canal e inicializa conversão */
			afec_channel_enable(AFEC_X, AFEC_X_CHANNEL);
			afec_start_software_conversion(AFEC_X);
		}
	}
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		.charlength = CONF_UART_CHAR_LENGTH,
		.paritytype = CONF_UART_PARITY,
		.stopbits = CONF_UART_STOP_BITS,
	};

	/* Configure console UART. */
	stdio_serial_init(CONF_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	setbuf(stdout, NULL);
}

static void BUT_init(void) {
	/* configura prioridae */
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4);

	/* conf botão como entrada */
	pio_configure(BUT_PIO, PIO_INPUT, BUT_PIO_PIN_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT_PIO, BUT_PIO_PIN_MASK, 60);
	pio_enable_interrupt(BUT_PIO, BUT_PIO_PIN_MASK);
	pio_handler_set(BUT_PIO, BUT_PIO_ID, BUT_PIO_PIN_MASK, PIO_IT_FALL_EDGE , but_callback);
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/


int main(void) {
	/* Initialize the SAM system */
	sysclk_init();
	board_init();

	/* Initialize the console uart */
	configure_console();
	xQueueADC = xQueueCreate(5, sizeof( adcData ));
	/* Create task to control oled */
	if (xTaskCreate(task_oled, "oled", TASK_OLED_STACK_SIZE, NULL, TASK_OLED_STACK_PRIORITY, NULL) != pdPASS) {
	  printf("Failed to create oled task\r\n");
	}
	/* Create task to handler LCD */
	if (xTaskCreate(task_adc, "adc", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test adc task\r\n");
	}
	/* Create task to handler LCD */
	if (xTaskCreate(task_adc_y, "adc", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create test adc task\r\n");
	}


	/* Start the scheduler. */
	vTaskStartScheduler();
	
	
  /* RTOS não deve chegar aqui !! */
	while(1){}

	/* Will only get here if there was insufficient memory to create the idle task. */
	return 0;
}
