#ifdef SEMIHOSTING
#include <stdio.h>
#endif
#include <string.h>

#include "parts/m55800/reg_m55800.h"

#define ETH_BASE 0x10000000
#define ETH_TX_BUFFER 0x4000
#define ETH_TX_MASK 0x1FFF
#define ETH_RX_BUFFER 0x6000
#define ETH_RX_MASK 0x1FFF

#define ETH_GAR 0x80 /* to 0x83 */
#define ETH_SMR 0x84 /* to 0x87 */
#define ETH_SHAR 0x88 /* to 0x8D */
#define ETH_SIPR 0x8E /* to 0x91 */

#define ETH_C0_CR 0x00
#define ETH_C0_CR_SYS_INIT 0x01
#define ETH_C0_CR_SOCK_INIT 0x02
#define ETH_C0_CR_CONNECT 0x04
#define ETH_C0_CR_CLOSE 0x10
#define ETH_C0_CR_SEND 0x20
#define ETH_C0_CR_RECV 0x40
#define ETH_C0_CR_SWRESET 0x80

#define ETH_C0_SOPR 0xA1
#define ETH_SOPR_CLOSED 0x0
#define ETH_SOPR_SOCK_STREAM 0x01
#define ETH_SOPR_SOCK_DGRAM 0x02

#define ETH_RMSR 0x95
#define ETH_RMSR_CH0_8KB 0x03

#define ETH_TMSR 0x96
#define ETH_TMSR_CH0_8KB 0x03

#define ETH_C0_TW_PR 0x40
#define ETH_C0_TA_PR 0x18
#define ETH_C0_TR_PR 0x44

#define ETH_C0_DIR 0xA8 /* to 0xAB */
#define ETH_C0_DPR 0xAC /* to 0xAD */
#define ETH_C0_SPR 0xA3 /* to 0xAF */

#define DNS_HEADER_ID 0x10AB
#define DNS_HEADER_QR ((0x0 << 15) & 0x8000) /* query */
#define DNS_HEADER_OPCODE ((0x0 << 11) & 0x7800) /* standard query */
#define DNS_HEADER_AA ((0x0 << 10) & 0x0400) /* not authoritative */
#define DNS_HEADER_TC ((0x0 << 9) & 0x0200) /* not truncated */
#define DNS_HEADER_RD ((0x1 << 8) & 0x0100) /* recursion desired */
#define DNS_HEADER_RA ((0x0 << 7) & 0x0080) /* no recursion available */
#define DNS_HEADER_Z ((0x0 << 4) & 0x0070)
#define DNS_HEADER_RCODE (0x0 & 0x000F)
#define DNS_HEADER_QDCOUNT 0x01 /* 1 query */
#define DNS_HEADER_ANCOUNT 0x00
#define DNS_HEADER_NSCOUNT 0x00
#define DNS_HEADER_ARCOUNT 0x00
#define DNS_QUERY_QNAME "\3www\6google\3com" /* string literal automatically has \0 at the end */
#define DNS_QUERY_QTYPE 0x01 /* get the IPv4 address */
#define DNS_QUERY_QCLASS 0x01 /* we are on the Internet */

#define GATEWAY 0xC0A80001 // 192.168.0.1
#define SUBNET 0xFFFFFF00 // 255.255.255.0
#define MACADDR 0x0019DB4D4DB2 // 00:19:DB:4D:4D:B2 (stolen from Aditya-Desktop)
#define IPADDR 0xC0A80040 // 192.168.0.64

unsigned short endianSwap16(unsigned short x);

unsigned int endianSwap32(unsigned int x);

unsigned long long endianSwap64(unsigned long long x);

void initialize(unsigned int gateway, unsigned int subnet, unsigned long long macaddr, unsigned int ipaddr);

void setup();

int main()
{
    unsigned char *ptrc;
    unsigned short *ptrs;
    unsigned int *ptri;
    
    setup();
    
    initialize(GATEWAY, SUBNET, MACADDR, IPADDR);

    ptrc = ETH_BASE + ETH_C0_SOPR;
    *ptrc = ETH_SOPR_SOCK_DGRAM;

    ptri = ETH_BASE + ETH_C0_DIR;
    memmove(ptri, ETH_BASE + ETH_GAR, 4);

    ptrs = ETH_BASE + ETH_C0_DPR;
    *ptrs = 53;

    ptrs = ETH_BASE + ETH_C0_SPR;
    *ptrs = 44532;

    ptrc = ETH_BASE + ETH_C0_CR;
    *ptrc = ETH_C0_CR_SOCK_INIT;

    ptrs = ETH_BASE + ETH_TX_BUFFER;
    *ptrs = endianSwap16(DNS_HEADER_ID);

    /*++ptrs;
    *ptri += 2;
    
    *ptrs = endianSwap16(DNS_HEADER_QR + DNS_HEADER_OPCODE + DNS_HEADER_AA + DNS_HEADER_TC + DNS_HEADER_RD + DNS_HEADER_RA + DNS_HEADER_Z + DNS_HEADER_RCODE);

    ++ptrs;
    *ptri += 2;
    
    *ptrs = endianSwap16(DNS_HEADER_QDCOUNT);

    ++ptrs;
    *ptri += 2;

    *ptrs = endianSwap16(DNS_HEADER_ANCOUNT);

    ++ptrs;
    *ptri += 2;
    
    *ptrs = endianSwap16(DNS_HEADER_NSCOUNT);

    ++ptrs;
    *ptri += 2;
    
    *ptrs = endianSwap16(DNS_HEADER_ARCOUNT);

    ++ptrs;
    *ptri += 2;
    
    ptrc = ptrs;
    strcpy(ptrc, DNS_QUERY_QNAME);

    ptrs += strlen(DNS_QUERY_QNAME) % 2 == 0 ? strlen(DNS_QUERY_QNAME) + 2 : strlen(DNS_QUERY_QNAME) + 1;
    *ptri += strlen(DNS_QUERY_QNAME) % 2 == 0 ? strlen(DNS_QUERY_QNAME) + 2 : strlen(DNS_QUERY_QNAME) + 1;
    
    *ptrs = endianSwap16(DNS_QUERY_QTYPE);

    ++ptrs;
    *ptri += 2;
    
    *ptrs = endianSwap16(DNS_QUERY_QCLASS);*/

    return 0;
}

unsigned short endianSwap16(unsigned short x)
{
    return (x << 8) | (x >> 8);
}

unsigned int endianSwap32(unsigned int x)
{
    return (x >> 24) | 
           ((x << 8) & 0x00FF0000) |
           ((x >> 8) & 0x0000FF00) |
           (x << 24);
}

unsigned long long endianSwap64(unsigned long long x)
{
    return (x >> 56) | 
           ((x << 40) & 0x00FF000000000000) |
           ((x << 24) & 0x0000FF0000000000) |
           ((x << 8)  & 0x000000FF00000000) |
           ((x >> 8)  & 0x00000000FF000000) |
           ((x >> 24) & 0x0000000000FF0000) |
           ((x >> 40) & 0x000000000000FF00) |
           (x << 56);
}

void initialize(unsigned int gateway, unsigned int subnet, unsigned long long macaddr, unsigned int ipaddr)
{
    unsigned int *ptri;
    unsigned short *ptrs;
    unsigned char *ptrc;

    /* convert to big-endian for the Wiznet module */
    gateway = endianSwap32(gateway);
    subnet = endianSwap32(subnet);
    macaddr = endianSwap64(macaddr) & 0x0000FFFFFFFFFFFF;
    ipaddr = endianSwap32(ipaddr);
    
    /* now set up the Ethernet module */
    ptri = ETH_BASE + ETH_GAR;
    *ptri = gateway;

    ptri = ETH_BASE + ETH_SMR;
    *ptri = subnet;

    ptri = ETH_BASE + ETH_SHAR;
    *ptri = macaddr & 0x0000FFFFFFFF;
    ptrs = ETH_BASE + ETH_SHAR + 4;
    *ptrs = macaddr >> 32;

    ptrs = ETH_BASE + ETH_SIPR;
    *ptrs = ipaddr & 0x0000FFFF;
    ++ptrs;
    *ptrs = ipaddr >> 16;

    // execute sys_init
    ptrc = ETH_BASE + ETH_C0_CR;
    *ptrc = ETH_C0_CR_SYS_INIT;

    ptrc = ETH_BASE + ETH_RMSR;
    *ptrc = ETH_RMSR_CH0_8KB;

    ptrc = ETH_BASE + ETH_TMSR;
    *ptrc = ETH_TMSR_CH0_8KB;
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

