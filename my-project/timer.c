/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2015 Piotr Esden-Tempski <piotr@esden.net>
 * Copyright (C) 2015 Jack Ziesing <jziesing@gmail.com>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <stdlib.h>
#include <tim_hdr.h>

static struct tim_cb_str *tim_cb_str_p;

static int tim_init_handler(struct tim_cb_str *cb_str) 
{
	if(cb_str->data_size <= MAX_CB_DATA_LEN && cb_str->tim_cb != NULL) {
		tim_cb_str_p = cb_str;
	} else {
		DBG_PRINT("the initializiation failed, too long data \n");
		DBG_LED(0x5);
		return E_GER;
	}

	return E_OK;
}

void clock_setup(void)
{
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
}

void tim_setup(uint16_t period, uint16_t frequancy,struct tim_cb_str *cb_str)
{
	tim_init_handler(cb_str);
	/* Enable TIM2 clock. */
	rcc_periph_clock_enable(RCC_TIM2);

	/* Enable TIM2 interrupt. */
	nvic_enable_irq(NVIC_TIM2_IRQ);

	/* Reset TIM2 peripheral to defaults. */
	rcc_periph_reset_pulse(RST_TIM2);

	/* Timer global mode:
	 * - No divider
	 * - Alignment edge
	 * - Direction up
	 * (These are actually default values after reset above, so this call
	 * is strictly unnecessary, but demos the api for alternative settings)
	 */
	timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);

	/*
	 * Please take note that the clock source for STM32 timers
	 * might not be the raw APB1/APB2 clocks.  In various conditions they
	 * are doubled.  See the Reference Manual for full details!
	 * In our case, TIM2 on APB1 is running at double frequency, so this
	 * sets the prescaler to have the timer run at 5kHz
	 */
	timer_set_prescaler(TIM2, ((rcc_apb1_frequency * 2) / frequancy));
	
	DBG_PRINT("the freq Is %u \n", rcc_apb1_frequency);
	/* Disable preload. */
	timer_disable_preload(TIM2);
	timer_continuous_mode(TIM2);

	/* count full range, as we'll update compare value continuously */
	timer_set_period(TIM2, period);

	/* Set the initual output compare value for OC1. */
	//timer_set_oc_value(TIM2, TIM_OC1, t_val);

	/* Counter enable. */
	//timer_enable_counter(TIM2);

	/* Enable Channel 1 compare interrupt to recalculate compare values */
	timer_enable_irq(TIM2, TIM_DIER_CC1IE);
}

void tim_enable(void) {
	timer_enable_counter(TIM2);
}

void tim_disable(void) {
	timer_disable_counter(TIM2);
}

void tim_setup_master_trig(uint16_t period, uint16_t frequancy)
{
	/* Set up the timer TIM2 for injected sampling */
	uint32_t timer;

	timer   = TIM2;

	rcc_periph_clock_enable(RCC_TIM2);

	/* Time Base configuration */
    rcc_periph_reset_pulse(RST_TIM2);
	
    timer_set_mode(timer, TIM_CR1_CKD_CK_INT,
	    TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_set_period(timer,period);
	timer_set_prescaler(timer, ((rcc_apb1_frequency * 2) / frequancy));
    timer_set_clock_division(timer, 0x0);

	timer_disable_preload(timer);
	timer_continuous_mode(timer);

    /* Generate TRGO on every update. */
    timer_set_master_mode(timer, TIM_CR2_MMS_UPDATE);
}

// thus you can regulate faze
int tim_set_oc_val(uint16_t freq) 
{
	/*
	* Get current timer value to calculate next
	* compare register value.
	*/
	uint16_t compare_time = timer_get_counter(TIM2);

	/* Calculate and set the next compare value. */
	uint16_t new_time = compare_time + freq;

	timer_set_oc_value(TIM2, TIM_OC1, new_time);

	return E_OK;
}

void tim2_isr(void)
{
	if (timer_get_flag(TIM2, TIM_SR_CC1IF)) {

		/* Clear compare interrupt flag. */
		timer_clear_flag(TIM2, TIM_SR_CC1IF);

		/* call the initialized handler */
		if (tim_cb_str_p != NULL) {
			tim_cb_str_p->tim_cb(&tim_cb_str_p->cb_data);
		} else {
			DBG_PRINT("the callback struct is not initialized; the timer is stoped\n");
			DBG_LED(6);
			timer_disable_counter(TIM2);
		}
	}
}
