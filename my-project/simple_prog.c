#include <led_hdr.h>
#include <general_defs.h>
#include <math.h>

int main(void)
{
	leds_init();
	leds_write(1);
	initialise_monitor_handles();
	while (1) {
	}

	return 0;
}