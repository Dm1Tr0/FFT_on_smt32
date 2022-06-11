#ifndef ADC_HDR
#define ADC_HDR

#define AMT_OF_CHAN_USED 1
#define MAX_CB_DATA_LEN 1000

struct adc_cb_data {
	uint8_t abstract_data[MAX_CB_DATA_LEN];
};

struct adc_cb_str {
	void (*adc_cb)(struct adc_cb_data *data);
	struct adc_cb_data cb_data;
	uint32_t data_size;
};

void adc_init_extern_trig(struct adc_cb_str * cb_str);

void adc_init_cb(struct adc_cb_str *cb_str);

uint16_t adc_acquire(void);

void adc_init(void);

#endif
