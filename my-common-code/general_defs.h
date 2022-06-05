#include <stdlib.h>

#ifndef GENERAL_DEFS
#define GENERAL_DEFS 

#if defined(ENABLE_SEMIHOSTING) && (ENABLE_SEMIHOSTING == 1)
#include <stdio.h>
extern void initialise_monitor_handles(void);

#define DBG_PRINT( ... ) ({ \
    if ("__VA_ARGS__") {    \
        printf(__VA_ARGS__);\
    } else {                \
        printf(__VA_ARGS__);\
    }                       \
})

#define DBG_LED(val)

#else

#define DBG_PRINT(val, ... ) 
#define DBG_LED(val)
#define initialise_monitor_handles() 

#endif

enum err_vals {
    E_OK,
    E_GER   //general error
};

#endif
