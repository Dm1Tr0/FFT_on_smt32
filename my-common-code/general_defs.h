#include <stdlib.h>

#ifndef GENERAL_DEFS
#define GENERAL_DEFS

#ifndef DBG_LVL

#define DBG_PRINT(val, ... )  
#define DBG_LED(val)

#endif 

#if (ENABLE_SEMIHOSTING == 1)
#include <stdio.h>
extern void initialise_monitor_handles(void);
#endif

enum err_vals {
    E_OK,
    E_GER   //general error
};

#endif
