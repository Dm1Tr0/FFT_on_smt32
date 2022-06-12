#include <stdlib.h>

#ifndef GENERAL_DEFS
#define GENERAL_DEFS 

#if defined(ENABLE_SEMIHOSTING) && (ENABLE_SEMIHOSTING == 1)
#include <stdio.h>
#include <inttypes.h>
extern void initialise_monitor_handles(void);

#define DBG_PRINT( ... ) ({ \
        printf(__VA_ARGS__);\
})

static inline void dbg_array_print(uint16_t *ptr, uint32_t lenth)
{
	char *ptr_8 = (char *)ptr;
	for (uint32_t i = 0; i < lenth; i++) {
		DBG_PRINT("%"PRIu16" cnt = %d \n", ptr[i], i);
	}
}

#define DBG_LED(val)

#else

#define DBG_PRINT(val, ... ) 
#define DBG_LED(val)
#define initialise_monitor_handles() 
#define dbg_array_print(ptr, len)

#endif

enum err_vals {
    E_OK,
    E_GER   //general error
};

#endif
