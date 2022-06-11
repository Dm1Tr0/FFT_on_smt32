#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/cm3/nvic.h>
#include <general_defs.h>
#include <inttypes.h>
#include <led_hdr.h>
#include <adc_hdr.h>
#include <tim_hdr.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define AMNT 100
#define CHNAN_AMTN 1
uint16_t samples[AMNT];
int32_t samples_amnt = AMNT;

struct adc_cb_str *adc_str_p;

void adc_init_cb(struct adc_cb_str *cb_str)
{
	if(cb_str->adc_cb != NULL && cb_str->data_size <= MAX_CB_DATA_LEN ) {
		adc_str_p = cb_str;
	} else {
		DBG_PRINT(" %s the cb_does not initialized the cb_str->adc_cb = %p && cb_str->data_size = %"PRIu32" \n", __func__, cb_str->adc_cb, cb_str->data_size);
	}
}

void adc_init_extern_trig(struct adc_cb_str * cb_str)
{
	// // * Sensor is connected to ADC12IN9 -- ADC1 and ADC2 (pin is common for them), input 9
	// // * By opening STM32F407_Datasheet_(DS8626).pdf at page 50, we can see ADC12_IN9 is
	// //   an additional function of PB1 pin
	// * So set PB1 to analog
	adc_init_cb(cb_str);
	rcc_periph_clock_enable(RCC_GPIOB);
	// set ospeed to lowest to minimize noise and power use as application note recommends
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, 1 << 1);
	// now set this pin to analog mode
	gpio_mode_setup(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, 1 << 1);

	// // int i;
	rcc_periph_clock_enable(RCC_ADC1);

	/* Make sure the ADC doesn't run during config. */
	adc_power_off(ADC1);

	/* We configure everything for one single timer triggered injected conversion. */
	adc_disable_scan_mode(ADC1);
	adc_set_single_conversion_mode(ADC1);

	/* We can only use discontinuous mode on either the regular OR injected channels, not both */
	adc_disable_discontinuous_mode_injected(ADC1);
	adc_enable_discontinuous_mode_regular(ADC1,1);

	/* We want to start the injected conversion with the TIM2 TRGO */
	adc_enable_external_trigger_regular(ADC1, ADC_CR2_EXTSEL_TIM2_TRGO,  ADC_CR2_EXTEN_BOTH_EDGES);
	adc_set_right_aligned(ADC1);

	DBG_PRINT("1 the adc_cr1 EOC = %x \n",ADC_CR1(ADC1) & ADC_CR1_EOCIE);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC);

	nvic_enable_irq(NVIC_ADC_IRQ);
	adc_enable_eoc_interrupt(ADC1);

	DBG_PRINT("1 the adc_cr1 EOC = %x \n",ADC_CR1(ADC1) & ADC_CR1_EOCIE);

	adc_power_on(ADC1);

	/* Wait for ADC starting up. */
	for (int i = 0; i < 800000; i++)    /* Wait a bit. */
		__asm__("nop");
}

void adc_init(void)
{
	// * Sensor is connected to ADC12IN9 -- ADC1 and ADC2 (pin is common for them), input 9
	// * By opening STM32F407_Datasheet_(DS8626).pdf at page 50, we can see ADC12_IN9 is
	//   an additional function of PB1 pin
	// * So set PB1 to analog
	rcc_periph_clock_enable(RCC_GPIOB);
	// set ospeed to lowest to minimize noise and power use as application note recommends
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, 1 << 1);
	// now set this pin to analog mode
	gpio_mode_setup(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, 1 << 1);

	rcc_set_ppre2(RCC_CFGR_PPRE_DIV_2);
	// Enable ADC clock
	rcc_periph_clock_enable(RCC_ADC1);

	// Set ADC prescaler to /8 (bits ADCPRE in ADC_CCR)
	adc_set_clk_prescale(ADC_CCR_ADCPRE_BY8);

	// Set ADC resolution to 12 bit
	adc_set_resolution(ADC1, ADC_CR1_RES_12BIT);
	// Set left-to-right data alignment in adc result register 

	uint8_t channels[CHNAN_AMTN];
	for (int i = 0; i < CHNAN_AMTN; i++) {
		adc_set_sample_time(ADC1, i, ADC_SMPR_SMP_480CYC);
		channels[i] = 9;	// set each element of group to channel 1
	}
	adc_set_regular_sequence(ADC1, CHNAN_AMTN, channels);

	// Configure End of Conversion (EOC) flag to be set after each channel in group
	// is converted. This will raise interrupt, where we read the conversion value
	adc_eoc_after_each(ADC1);

	// Enable scan mode to convert a group of channels
	adc_enable_scan_mode(ADC1);

	// Configure to perform a conversion of group of channels and stop
	adc_set_single_conversion_mode(ADC1);

	adc_enable_eoc_interrupt(ADC1);
	adc_enable_overrun_interrupt(ADC1);

	// Set ADC interrupts in NVIC
	nvic_set_priority(NVIC_ADC_IRQ, 10);
	nvic_enable_irq(NVIC_ADC_IRQ);

	// Enable ADC1. And wait some time to stabilize and calibrate itself
	adc_power_on(ADC1);

	for (int i = 0; i < 800000; i++)    /* Wait a bit. */
	__asm__("nop");
}

void adc_isr(void)
{
	if (adc_str_p != NULL) {
		adc_str_p->adc_cb(&adc_str_p->cb_data);
	} else {
		DBG_PRINT("%s the adc cb is not initialized \n");
	}
}

uint16_t adc_acquire(void)
{
	return adc_read_regular(ADC1);
}
