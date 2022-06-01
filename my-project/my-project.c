#include <led_hdr.h>
#include <tim_hdr.h>
#include <usb_hdr.h>
#include <string.h>

#define USB_DATA_BUF_LEN 100
char data_buffer[USB_DATA_BUF_LEN];

enum comands {    //        EXAMPLES
	START = '1',  //  START  n (where n amount of samples to read) no value instead of n means untill stop 
	STOP  = '2',  //  STOP   stop sampling
	READ  = '3'   //  READ   read the aray of samples
};

uint32_t request_handler(char *buff, int32_t len)
{
	if(len && buff != NULL) {

	} else {
		DBG_PRINT("unable to handle the usb reques the buf = NULL or lenth is invalid");
	}
	
	return E_OK;
}

static void usb_data_cb(struct usb_cb_data * cb_data)
{
	int32_t r_len = 0;
	(void) cb_data; // no extra data needed

	r_len = usb_read_data_packet(data_buffer, USB_DATA_BUF_LEN);
	leds_write(r_len);

	usb_write_data_packet(data_buffer,r_len);
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
