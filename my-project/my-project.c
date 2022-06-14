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

// discretisation clk = 1Mhz

#define USB_DATA_BUF_LEN  			100
#define MAX_AMNT_OF_SAMPL		 	1000
#define DEFAULT_D_WORDS_PER_ROUND	25

static char data_buffer[USB_DATA_BUF_LEN];
static uint16_t samples[MAX_AMNT_OF_SAMPL];
static int32_t samples_amnt;
uint8_t sampling_in_progress, stop_sampling, data_reading_in_progress;

uint32_t d_words_per_round = DEFAULT_D_WORDS_PER_ROUND;

enum comands {               //        definition
	START            = '1',  //  START  n  			(where n amount of samples to read) 
	READ_TD          = '2',  //  READ_TD   			read the aray of samples
	READ_DFT         = '3',  //  READ_     			read the resault of digital furie transformation aplied to the input signal
	READ_FFT         = '4',  //  READ_FFT  			read the resault of fest furie transformation aplied to the input signal
	SET_PER_ROUND    = '5',  //  SET_PER_ROUND	    set the ammount of samples(uint16_t) values per one usb data transfere to transfere
	GET_DEBUG_STATUS = '6'   //  GET_DEBUG_STATUS	send the debug status to via usbW
};

static int32_t set_d_words_per_round(char * buff)
{
	uint8_t per_round = atoi(buff);

	if (per_round <= 0 || per_round > DEFAULT_D_WORDS_PER_ROUND) {
		DBG_PRINT(" %s the prer_round value is invalid %d \n", per_round);
		return E_INV;
	}

	return per_round;
}

static int32_t pass_aray_via_usb(uint16_t *buff, uint32_t init_len) // lenth is not ment to be changed untill the entire data transfer ocured
{
	int32_t err = E_OK;
	static uint32_t len = 0;
	static uint32_t read = 0;
	static uint16_t *usb_buf = NULL;

	if (sampling_in_progress) {
		DBG_PRINT(" %s unable to handle reading reques sampling in progress \n", __func__);
		err = E_GER;
		goto _error;
	}

	if (!d_words_per_round) {
		DBG_PRINT(" %s the samples per round is not set \n", __func__);
		err = E_GER;
		goto _error;
	}
	
	if (!len || !usb_buf) { // to be sure that the lenth won't be changed until the data trasfere occured
		len = init_len;
		usb_buf = buff;
	}
	
	data_reading_in_progress = 1;

	read += d_words_per_round;

	usb_write_data_packet(usb_buf, read > len ? (d_words_per_round + len - read) * sizeof(uint16_t) : (d_words_per_round) * sizeof(uint16_t)); // as long as samples has uint16_t, we have to scale sample array size to char data type
	
	DBG_PRINT("%s the samples amnt %d, and read %d \n", __func__, len, read);

	if (read >= len) {
		data_reading_in_progress = 0;
		usb_buf = NULL;
		read = 0;
		len = 0;
		DBG_PRINT("%s the reading procidure ended \n", __func__);
	} else {
		usb_buf += d_words_per_round;
	}

_error:

	return err;
}

static int32_t get_signal(char *buff)
{
	uint32_t err = E_OK;
	char *ptr = NULL;

	DBG_PRINT("%s entered the START comand handler\n", __func__);
	if (data_reading_in_progress) {
		DBG_PRINT("%s unable to start sampling the reading in progress \n", __func__ );
		err = E_GER;
		goto _error;
	}
	samples_amnt = 0; // cleaning up the samples ammount
	if (buff[0] == ' ') {
		int32_t parsed_value = 0; 
		ptr = buff + 1; // pointer to second argument
		parsed_value = atoi(ptr);
		
		if (parsed_value > 0) { //dont forget to implement MIN_AMNT_OF_SAMPLS
			samples_amnt = parsed_value;
			memset(samples, 0, MAX_AMNT_OF_SAMPL);
			sampling_in_progress = 1;
			tim_enable();
		} else {
			DBG_PRINT(" %s the parsed amount of samples is invalid \n", __func__);
			err = E_GER;
			goto _error;
		}
	} else {
		DBG_PRINT(" %s unable to parse the comand that seems to look like start \n", __func__);
		err = E_GER;
		goto _error;
	}

_error:

	return err;
}

static uint32_t request_handler(char *buff, int32_t len)
{
	uint32_t err = E_OK;
	int32_t dw_per_rnd = E_INV;

	DBG_PRINT("entered the %s wit request '%s' \n", __func__, buff);

	if(len > 0 && buff != NULL) {
		switch (buff[0]) {
		case START:
			err = get_signal(buff + 1);
			if (err) {
				DBG_PRINT("%s signal get error ocured", __func__);
			}
			break;

		case READ_TD:
			err = pass_aray_via_usb(samples, samples_amnt);
			if (err) {
				DBG_PRINT("%s pass aray via usb error ocured", __func__);
			}
			break;

		case SET_PER_ROUND:
			dw_per_rnd = set_d_words_per_round(buff + 1);
			if (dw_per_rnd != E_INV ) {
				DBG_PRINT("%s double words per round is set to %d", __func__, dw_per_rnd);
				d_words_per_round = dw_per_rnd;
			}
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

static void adc_cb_func(struct adc_cb_data *cb_data)
{
	(void)cb_data;
	static int32_t cnt_samples = 0; 
	tim_disable(); // in order to avoid overrun
	if (!sampling_in_progress) {
		DBG_PRINT("possibly stucked inside of the adc interupt \n");
		return;
	}

	if(handle_adc_overrun()) {
		DBG_PRINT("the adc overrun \n");
		return;
	}

	if (samples_amnt == cnt_samples || stop_sampling) {
		DBG_PRINT("the sempling ended \n");
		sampling_in_progress = 0;
		stop_sampling = 0;
		cnt_samples = 0;
		return;
	}
	
	samples[cnt_samples] = adc_acquire();
	//DBG_PRINT("sample cnt = %"PRId32" sapmle restaut: %"PRIu32", true adc %"PRIu32"\n", cnt_samples, samples[cnt_samples], adc_acquire());
	cnt_samples++;
	tim_enable(); // in order to avoid overrun
}

int main(void)
{
	struct adc_cb_str adc_cb;
	struct usb_cb_str usb_cb;
	initialise_monitor_handles();
#ifndef DEBUG
	clock_setup();
#endif
	leds_init();
	leds_write(1);

	/* seting up the usb */
	memset(&usb_cb, 0, sizeof(struct usb_cb_str));
	usb_cb.usb_cb = usb_cb_func;
	usb_cb.data_size = 0; // no extra data
	usb_init(&usb_cb);

	/* seting up the timer wiht period 1 Mhz */
  	memset(&adc_cb, 0, sizeof(struct adc_cb_str));
  	adc_cb.adc_cb = adc_cb_func;
  	adc_cb.data_size = 0; // no extra data
	adc_init_extern_trig(&adc_cb);

#ifndef DEBUG
	tim_setup_master_trig(1, 50000000); 
#else
  	tim_setup_master_trig(1, 500); 
#endif
	
    leds_write(2);

	while (1) {
		static unsigned cnt;
		usbd_singl_poll();
		leds_write(cnt++);
	}

	return 0;
}   
