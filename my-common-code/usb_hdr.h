#include <stdint.h>
#include <general_defs.h>

#ifndef USB_HDR
#define USB_HDR

#define MAX_USB_CB_DATA_LEN 10000

struct usb_cb_data {
	uint8_t abstract_data[MAX_USB_CB_DATA_LEN];
};

struct usb_cb_str {
	void (*usb_cb)(struct usb_cb_data *data);
	struct usb_cb_data cb_data;
	uint32_t data_size;
};

int usb_read_data_packet(char *buff, uint32_t len);

int usb_write_data_packet(uint16_t *buff, uint32_t len);

void usb_init(struct usb_cb_str* cb_str);

void usbd_singl_poll(void);

#endif