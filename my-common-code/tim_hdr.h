#include <stdint.h>
#include <general_defs.h>

#ifndef TIM_HDR
#define TIM_HDR

#define MAX_CB_DATA_LEN 1000

struct tim_cb_data {
	uint8_t abstract_data[MAX_CB_DATA_LEN];
};

struct tim_cb_str {
	void (*tim_cb)(struct tim_cb_data *data);
	struct tim_cb_data cb_data;
	uint32_t data_size;
};

void tim_setup(uint16_t period, uint16_t frequancy,struct tim_cb_str *cb_str);

void tim_setup_master_trig(uint32_t period, uint32_t frequancy);

int  tim_set_oc_val(uint16_t freq);

void clock_setup(void);

void tim_disable(void);

void tim_enable(void);

#endif
