 #pragma once 

#include <stdint.h>
#include "sysio.h"
#include "dmd_limit.hpp"

#define UORB_PATH_LEN 6 //  /uorb/
#define UORB_FULL_PATH_LEN (UORB_PATH_LEN+VFS_ENTRY_NAME_MAX)

#define UORB_TP_SUFFIX_WIDTH 1
#define UORB_TP_MAX_INSTANCE 10

namespace UORB
{	

typedef  char            CHAR;
typedef  unsigned char   U8  ;
typedef  unsigned short  U16 ;
typedef  unsigned int    U32 ;
typedef  uint64_t        U64 ;
typedef  signed   char   S8  ;
typedef  signed   short  S16 ;
typedef  int             S32 ;

}
