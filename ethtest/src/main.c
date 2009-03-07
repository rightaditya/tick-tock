#ifdef SEMIHOSTING
#include <stdio.h>
#endif

#include "parts/m55800/reg_m55800.h"

#define ETH_BASE 0x10000000
#define ETH_C0_CR 0x00
#define ETH_C0_ISR 0x04
#define ETH_GAR 0x80
#define ETH_SMR 0x84
#define ETH_SHAR 0x88
#define ETH_SIPR 0x8E

#define GATEWAY 0x8EF4A19F // 142.244.161.159
#define SUBNET 0xFFFFFF00 // 255.255.255.0
#define MACADDR 0x111111111111
#define IPADDR 0xC0A80040 // 192.168.0.64

void setup();

int main()
{
    int *ptri;
    short *ptrs;
    char *ptrc;
    setup();

    // set the registers
    ptri = ETH_BASE + ETH_GAR;
    *ptri = GATEWAY;

    ptri = ETH_BASE + ETH_SMR;
    *ptri = SUBNET;

    ptri = ETH_BASE + ETH_SHAR;
    *ptri = MACADDR & 0x0000FFFFFFFF;
    ptrs = ETH_BASE + ETH_SHAR + 4;
    *ptrs = MACADDR >> 4;

    ptrs = ETH_BASE + ETH_SIPR;
    *ptrs = IPADDR & 0x0000FFFF;
    ++ptrs;
    *ptrs = IPADDR >> 4;

    // execute sys_init
    ptrc = ETH_BASE + ETH_C0_CR;
    *ptrc = 1;
    
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
    EBI_CSR2 |= EBI_NWS_6;
    EBI_CSR2 |= EBI_DBW_8;
    EBI_CSR2 |= ETH_BASE;
}
