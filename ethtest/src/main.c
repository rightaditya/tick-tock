#ifdef SEMIHOSTING
#include <stdio.h>
#endif
#include <string.h>

#include "parts/m55800/reg_m55800.h"
#include "iinchip_conf.h"
#include "types.h"
#include "w5300.h"

// TODO: constify
static uint8 IP[4] = {192, 168, 0, 101};
static uint8 GW[4] = {192, 168, 0, 1};
static uint8 SN[4] = {255, 255, 255, 0};
static uint8 MAC[6] = {0x00, 0x08, 0xDC, 0x00, 111, 200};

#define MAX_DNS_LENGTH 512
#define DNS_PORT 53
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

uint8 bigEndianFirst(uint16 val);

uint8 bigEndianSecond(uint16 val);

void setup();

int main()
{
    uint8 txMemConf = 64, rxMemConf = 64;
    uint8 dnsbuffer[MAX_DNS_LENGTH]; uint8 i = 0;
    uint16 *dnsbuffer16 = (uint16*)dnsbuffer;
    uint32 len;

    memset(dnsbuffer, 0, MAX_DNS_LENGTH);
    
    setup();

    iinchip_init();

    if (!sysinit(&txMemConf, &rxMemConf))
    {
	printf("Memory configuration error.\n");
	exit(1);
    }

    setMR(getMR() | MR_FS);

    setSHAR(MAC);
    setGAR(GW);
    setSUBR(SN);
    setSIPR(IP);

    // TODO: this is for testing only, get rid of it
    getSHAR(MAC);
    getGAR(GW);
    getSUBR(SN);
    getSIPR(IP);

    socket(0, Sn_MR_UDP, DNS_PORT << 10, 0);

    dnsbuffer16[i++] = DNS_HEADER_ID;

    dnsbuffer16[i++] = DNS_HEADER_QR + DNS_HEADER_OPCODE + DNS_HEADER_AA + DNS_HEADER_TC + DNS_HEADER_RD + DNS_HEADER_RA + DNS_HEADER_Z + DNS_HEADER_RCODE;

    dnsbuffer16[i++] = DNS_HEADER_QDCOUNT;

    dnsbuffer16[i++] = DNS_HEADER_ANCOUNT;
    
    dnsbuffer16[i++] = DNS_HEADER_NSCOUNT;

    dnsbuffer16[i++] = DNS_HEADER_ARCOUNT;

    strcpy(&(dnsbuffer[i <<= 1]), DNS_QUERY_QNAME);
    i += strlen(DNS_QUERY_QNAME) + 1;

    dnsbuffer[i++] = bigEndianFirst(DNS_QUERY_QTYPE);
    dnsbuffer[i++] = bigEndianSecond(DNS_QUERY_QTYPE);

    dnsbuffer[i++] = bigEndianFirst(DNS_QUERY_QCLASS);
    dnsbuffer[i++] = bigEndianSecond(DNS_QUERY_QCLASS);

    sendto(0, dnsbuffer, i, GW, DNS_PORT);

    while (1)
    {
	if ((len = getSn_RX_RSR(0)) > 0)
	{
	    len = recvfrom(0, dnsbuffer, len, GW, DNS_PORT);
	}
    }

    return 0;
}

void setup()
{
    EBI_CSR2 = 0x00000000;
    EBI_CSR2 |= EBI_CSEN;
    EBI_CSR2 |= EBI_BAT_BYTE_SELECT;
    EBI_CSR2 |= EBI_TDF_1;
    EBI_CSR2 |= EBI_PAGES_1M;
    EBI_CSR2 |= EBI_WSE;
    EBI_CSR2 |= EBI_NWS_8;
    EBI_CSR2 |= EBI_DBW_16;
    EBI_CSR2 |= __DEF_IINCHIP_MAP_BASE__;
}

uint8 bigEndianFirst(uint16 val)
{
    return val & 0x00FF;
}

uint8 bigEndianSecond(uint16 val)
{
    return val >> 8;
}

