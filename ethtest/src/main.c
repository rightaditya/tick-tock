#ifdef SEMIHOSTING
#include <stdio.h>
#endif

#include "parts/m55800/reg_m55800.h"

#define ETH_BASE 0x10000000

void setup();

int main()
{
    setup();
    
    return 0;
}

void setup()
{
    EBI_CSR2 = 0x00000000;
    EBI_CSR2 |= EBI_CSEN;
    EBI_CSR2 |= EBI_BAT_BYTE_WRITE;
    EBI_CSR2 |= EBI_TDF_1;
    EBI_CSR2 |= EBI_PAGES_1M;
    EBI_CSR2 |= EBI_WSE;
    EBI_CSR2 |= EBI_NWS_3;
    EBI_CSR2 |= EBI_DBW_8;
    EBI_CSR2 |= ETH_BASE;
}
