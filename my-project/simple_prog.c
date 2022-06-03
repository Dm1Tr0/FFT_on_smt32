#include <led_hdr.h>
#include <general_defs.h>

int main(void)
{
	leds_init();
	leds_write(1);
	initialise_monitor_handles();
	
	printf("kurva \n");
	while (1) {
	}

	return 0;
}