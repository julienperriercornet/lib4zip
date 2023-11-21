#pragma once


// Use either encoder but not both at the same time.
//#define ARITH64
#define ARITH32


#ifdef ARITH64
#include "arith64.h"
#else
#ifdef ARITH32
#include "arith32.h"
#endif
#endif
