#ifdef SEMIHOSTING
#include <stdio.h>
#endif

#include "parts/m55800/reg_m55800.h"

#define ETH_BASE 0x10000000
#define ETH_C0_CR 0x00
#define ETH_C0_CR_SYS_INIT 0x01
#define ETH_C0_CR_SOCK_INIT 0x02
#define ETH_C0_CR_CONNECT 0x04
#define ETH_C0_ISR 0x04
#define ETH_C0_ISR_INIT_OK 0x01
#define ETH_GAR 0x80
#define ETH_SMR 0x84
#define ETH_SHAR 0x88
#define ETH_SIPR 0x8E

#define GATEWAY 0x0100A8C0 // 192.168.0.1 backwards for big-endian Wiznet
#define SUBNET 0x00FFFFFF // 255.255.255.0 backwards for big-endian Wiznet
#define MACADDR 0xB24D4DDB1900 // 00:19:DB:4D:4D:B2 (stolen from Aditya-Desktop) backwards for big-endian wiznet
#define IPADDR 0x4000A8C0 // 192.168.0.64 backwards for big-endian Wiznet

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
    *ptrs = MACADDR >> 32;

    ptrs = ETH_BASE + ETH_SIPR;
    *ptrs = IPADDR & 0x0000FFFF;
    ++ptrs;
    *ptrs = IPADDR >> 16;

    // execute sys_init
    ptrc = ETH_BASE + ETH_C0_CR;
    *ptrc |= ETH_C0_CR_SYS_INIT;

    /*ptri = ETH_BASE + ETH_C0_ISR;
    if (*ptri & ETH_C0_ISR_INIT_OK)
        printf("Initialized OK.\n");
    else
        printf("Couldn't initialize!\n");*/
    
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

