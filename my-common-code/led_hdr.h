#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <stdint.h>

#ifndef GPIO_HDR
#define GPIO_HDR

#define GREEN     GPIO12
#define YELOW     GPIO13 
#define BLUE      GPIO15
#define RED       GPIO14
#define LED_FIRST 12

void leds_init(void);

void leds_set(uint16_t data);

void leds_write(uint16_t data);

#endif
