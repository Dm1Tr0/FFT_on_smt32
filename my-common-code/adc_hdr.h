#ifndef ADC_HDR
#define ADC_HDR

#define AMT_OF_CHAN_USED 1

void adc_init(void);

uint16_t adc_acquire(void);

#endif