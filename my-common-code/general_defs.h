#include <stdlib.h>

#ifndef GENERAL_DEFS
#define GENERAL_DEFS

#ifndef DBG_LVL

#define DBG_PRINT(val, ... )  
#define DBG_LED(val)

#endif 

enum err_vals {
    E_OK,
    E_GER   //general error
};

#endif
