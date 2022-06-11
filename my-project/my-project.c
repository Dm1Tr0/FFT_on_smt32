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
static int32_t samples_amnt = 100;
uint8_t sampling_in_progress, stop_sampling, reading_in_progres;

enum comands {    //        EXAMPLES
	START    = '1',  //  START  n  (where n amount of samples to read) no value instead of n means untill stop 
	READ_TD  = '2',  //  READ_TD   read the aray of samples
	READ_DFT = '3',  //  READ_     read the resault of digital furie transformation aplied to the input signal
	READ_FFT = '4',   //  READ_FFT  read the resault of fest furie transformation aplied to the input signal
	R_TEST   = '5'

};

static uint32_t request_handler(char *buff, int32_t len)
{
	char *ptr = NULL;
	uint32_t err = E_OK;

	leds_write(6);

	if(len > 0 && buff != NULL) {
		switch (buff[0]) {
		case START:
			if (reading_in_progres) {
				DBG_PRINT("%s unable to start sampling the reading in progress", __func__ );
				break;
			}
			samples_amnt = 0; // cleaning up the samples ammount
			if (buff[1] == ' ') {
				int32_t parsed_value = 0; 
				ptr = buff + 2; // pointer to second argument
				parsed_value = atoi(ptr);
				
				if (parsed_value > 0) { //dont forget to implement MIN_AMNT_OF_SAMPLS
					samples_amnt = parsed_value;
					memset(samples, 0, MAX_AMNT_OF_SAMPL);
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
			if (sampling_in_progress) {
				DBG_PRINT(" %s unable to handle reading reques sampling in progress", __func__);
				break;
			}
			#define READ_CNT 50  // yeah looks bed :) but unfortunatly I unable to pass more then 50 bytes at a time. so this is the bes way I found to solve the problem
			static int32_t read = 0;
			static uint16_t *sample_p = samples;
			
			reading_in_progres = READ_CNT;
			read += READ_CNT;

			dbg_array_print(sample_p, samples_amnt - read < 0 ? READ_CNT + samples_amnt - read : READ_CNT);
			usb_write_data_packet(sample_p, samples_amnt - read < 0 ? READ_CNT + samples_amnt - read : READ_CNT); // as long as samples has uint16_t, we have to scale sample array size to char data type
			
			if (read >= samples_amnt) {
				reading_in_progres = 0;
				sample_p = samples;
			} else {
				sample_p = read + samples;
			}

			#undef READ_CNT
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
	static int32_t cnt_samples = 0; 

	if (samples_amnt == cnt_samples || stop_sampling) {
		tim_disable();
		sampling_in_progress = 0;
		stop_sampling = 0;
		cnt_samples = 0;
		return;
	}

	samples[cnt_samples] = 1;
	cnt_samples++;
	DBG_PRINT("samples left %"PRIu32" \n", samples[cnt_samples]);
	
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
  	tim_setup(1, 5000000, &tim_cb); 
	
	adc_init();
    leds_write(2);
	tim_enable();
	while (1) {
		adc_acquire();
		usbd_singl_poll();
	}

	return 0;
}   
