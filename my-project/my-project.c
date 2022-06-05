#include <libopencm3/stm32/flash.h>
#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/cm3/nvic.h>
#include <general_defs.h>
#include <inttypes.h>
#include <led_hdr.h>
#include <tim_hdr.h>
#include <usb_hdr.h>
#include <adc_hdr.h>
#include <string.h>
#include <stdint.h>

#define USB_DATA_BUF_LEN 100
#define MIN_AMNT_OF_SAMPL 128
#define MAX_AMNT_OF_SAMPL 1000

static char data_buffer[USB_DATA_BUF_LEN];
static uint16_t samples[MAX_AMNT_OF_SAMPL];
static uint32_t samples_amnt;
uint8_t sampling_in_progress, stop_sampling;

enum comands {    //        EXAMPLES
	START    = '1',  //  START  n (where n amount of samples to read) no value instead of n means untill stop 
	READ_TD  = '2',  //  READ_TD   read the aray of samples
	READ_DFT = '3',   
	READ_FFT = '4'   //  READ_TD
};

static uint32_t request_handler(char *buff, int32_t len)
{
	char *ptr = NULL;
	uint32_t err = E_OK;

	leds_write(6);

	if(len > 0 && buff != NULL) {
		switch (buff[0]) {
		case START:
			if (buff[1] == ' ') {
				int32_t parsed_value = 0; 
				ptr = buff + 2; // pointer to second argument
				parsed_value = atoi(ptr);
				if (parsed_value > 0) { //dont forget to implement MIN_AMNT_OF_SAMPLS
					leds_write(parsed_value);
					samples_amnt = parsed_value;
					tim_enable();
				} else {
					DBG_PRINT(" %s the parsed amount of samples is invalid \n", __func__);
					err = E_GER;
					break;
				}
			} else {
				DBG_PRINT(" %s unable to parse the comand that seems to look like start \n", __func__);
				err = E_GER;
				break;
			}
			break;

		case READ_TD:
			usb_write_data_packet((char *)samples, 1); // dont forget to set the amnt of symbols to wr
			memset(samples, 0, MAX_AMNT_OF_SAMPL);
			break;
			
		default:
			DBG_PRINT(" %s unsuported command \n", __func__);
			break;
		}
	} else {
		DBG_PRINT("unable to handle the usb reques the buf = NULL or lenth is invalid");
	}
	
	return err;
}

static void usb_cb_func(struct usb_cb_data * cb_data)
{
	int32_t r_len = 0;
	(void) cb_data; // no extra data needed

	r_len = usb_read_data_packet(data_buffer, USB_DATA_BUF_LEN);

	request_handler(data_buffer, r_len);

	memset(data_buffer, 0, r_len);
}

static void tim_cb_func(struct tim_cb_data *cb_data)
{
	(void)cb_data;
	sampling_in_progress = 1;

	if (samples_amnt-- == 0 || stop_sampling) {
		tim_disable();
		sampling_in_progress = 0;
		stop_sampling = 0;
		return;
	}

	DBG_PRINT("samples left %"PRIu32" \n", samples_amnt);
	
}

int main(void)
{
	struct tim_cb_str tim_cb;
	struct usb_cb_str usb_cb;
	initialise_monitor_handles();
	clock_setup();
	leds_init();
	leds_write(1);

    
	/* seting up the usb */
	memset(&usb_cb, 0, sizeof(struct usb_cb_str));
	usb_cb.usb_cb = usb_cb_func;
	usb_cb.data_size = 0; // no extra data
	usb_init(&usb_cb);

	/* seting up the timer wiht period 1 Mhz */
  	memset(&tim_cb, 0, sizeof(struct tim_cb_str));
  	tim_cb.tim_cb = tim_cb_func;
  	tim_cb.data_size = 0; // no extra data
  	tim_setup(1, 1000000, &tim_cb); 
	
	adc_init();
    leds_write(2);

	while (1) {
		usbd_singl_poll();
	}

	return 0;
}   
