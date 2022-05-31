#include <led_hdr.h>
#include <tim_hdr.h>
#include <string.h>

static void test_led_count(struct tim_cb_data *cb_data)
{
	static uint8_t cnt = 0;

	leds_write(cnt++);
	/* data */
}

int main(void)
{
	struct tim_cb_str tim_cb;

	leds_init();
	tim_clock_setup();

	leds_write(1);

	memset(&tim_cb, 0, sizeof(struct tim_cb_str));
	tim_cb.tim_cb = test_led_count;
	tim_cb.data_size = 0; // no extra data
	tim_init_handler(&tim_cb);
	
	leds_write(2);

    tim_setup(5000); // one second
    
	leds_write(3);
	while (1);
	leds_write(4);
	return 0;
}
