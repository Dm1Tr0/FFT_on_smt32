#include <led_hdr.h>
#include <tim_hdr.h>
#include <usb_hdr.h>
#include <string.h>
#include <general_defs.h>

#define USB_DATA_BUF_LEN 100
#define MIN_AMNT_OF_SAMPL 128
#define MAX_AMNT_OF_SAMPL 1000

char data_buffer[USB_DATA_BUF_LEN];
char samples[MAX_AMNT_OF_SAMPL];
uint32_t samples_amnt;

enum comands {    //        EXAMPLES
	START    = '1',  //  START  n (where n amount of samples to read) no value instead of n means untill stop 
	READ_TD  = '2',   //  READ   read the aray of samples
	READ_DFT = '3',   //  STOP   stop sampling
	READ_FFT = '4' 
};

uint32_t request_handler(char *buff, int32_t len)
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
					samples[0] = *ptr;

				} else {
					DBG_PRINT(" %s the parsed amount of samples is invalid \n" __func__);
					err = E_GER;
					break;
				}
			} else {
				DBG_PRINT(" %s unable to parse the comand that seems to look like start \n" __func__);
				err = E_GER;
				break;
			}
			break;

		case READ_TD:
			usb_write_data_packet(samples, 1); // dont forget to set the amnt of symbols to wr
			memset(samples, 0, MAX_AMNT_OF_SAMPL);
			break;
			
		default:
			DBG_PRINT(" %s unsuported command \n" __func__);
			break;
		}
	} else {
		DBG_PRINT("unable to handle the usb reques the buf = NULL or lenth is invalid");
	}
	
	return err;
}

static void usb_data_cb(struct usb_cb_data * cb_data)
{
	int32_t r_len = 0;
	(void) cb_data; // no extra data needed

	r_len = usb_read_data_packet(data_buffer, USB_DATA_BUF_LEN);

	request_handler(data_buffer, r_len);

	memset(data_buffer, 0, r_len);
}

int main(void)
{
	struct usb_cb_str usb_cb;
	leds_init();
	leds_write(1);
    
	memset(&usb_cb, 0, sizeof(struct usb_cb_str));
	usb_cb.usb_cb = usb_data_cb;
	usb_cb.data_size = 0; // no extra data
	usb_init(&usb_cb);
	    
	leds_write(2);
	
	while (1) {
	    printf("oh kurva 0_0 \n");
		usbd_singl_poll();
	}

	return 0;
}

 /* working timer example
  *
  *
  *	static void test_led_count(struct tim_cb_data *cb_data)
  *	{
  *		static uint8_t cnt = 0; 
  *
  *		leds_write(cnt++);
  *	}
  *
  *
  *	int main(void)
  *	{
  *		struct tim_cb_str tim_cb;
  *
  * 	leds_init();
  *		tim_clock_setup();
  *
  *		leds_write(1);
  *
  *		memset(&tim_cb, 0, sizeof(struct tim_cb_str));
  *		tim_cb.tim_cb = test_led_count;
  *		tim_cb.data_size = 0; // no extra data
  *		tim_init_handler(&tim_cb);
  *		
  *		leds_write(2); 
  *
  *		tim_setup(5000); // one second
  *		
  *		leds_write(3);
  *		while (1);
  *		leds_write(4);
  *		return 0;
  *	}
  *
  */ 
