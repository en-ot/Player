#include <stdio.h>
#include "build_defs.h"

#include "version.h"
#include "firmware.h"


const char * firmware_version()
{
    return VERSION;
}

int firmware_datetime(char * buf, int len)
{
    return snprintf(buf, len, "%d.%02d.%d, %02d:%02d:%02d", 
                COMPUTE_BUILD_DAY, COMPUTE_BUILD_MONTH, COMPUTE_BUILD_YEAR,
                COMPUTE_BUILD_HOUR, COMPUTE_BUILD_MIN, COMPUTE_BUILD_SEC);
}
