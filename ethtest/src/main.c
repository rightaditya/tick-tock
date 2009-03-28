#ifdef SEMIHOSTING
#include <stdio.h>
#endif
#include <string.h>

#include "parts/m55800/reg_m55800.h"
#include "iinchip_conf.h"
#include "types.h"
#include "w5300.h"

#define MAX_DNS_LENGTH 512
#define DNS_PORT 53
#define DNS_CLIENT_PORT (DNS_PORT << 10)
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
#define DNS_QUERY_QTYPE 0x01 /* get the IPv4 address */
#define DNS_QUERY_QCLASS 0x01 /* we are on the Internet */

#define GOOGLE_DNS_NAME "\3www\6google\3com" /* string literal automatically has \0 at the end */
#define NTP_SERVER_DNS_NAME "\2ca\4pool\3ntp\3org"

#define MAX_NTP_LENGTH 48
#define NTP_PORT 123
#define NTP_CLIENT_PORT (NTP_PORT << 9)

#define HTTP_PORT 80
#define HTTP_CLIENT_PORT (HTTP_PORT << 9)

static uint8 IP[4] = {192, 168, 0, 101};
static uint8 GW[4] = {192, 168, 0, 1};
static uint8 SN[4] = {255, 255, 255, 0};
static uint8 MAC[6] = {0x00, 0x08, 0xDC, 0x00, 111, 200};

static uint8 GOOGLE_SERVER[4] = {74, 125, 155, 147};
static uint8 NTP_SERVER[4] = {206, 248, 142, 218};
static uint8 LAPTOP[4] = {192, 168, 0, 100};

typedef struct
{
    char low;
    char high;
} Weather;

uint8 first(uint16 val);

uint8 second(uint16 val);

uint16 endianSwap16(uint16 x);

uint32 endianSwap32(uint32 x);

unsigned int getCurrentTime(int tzShift);

void getIP(char* dnsName, uint8 *IP);

Weather getWeather();

void setup();

void loopback(SOCKET s, uint16 port, uint8* buf, uint16 mode)
{
    uint32 len;
    uint8 destip[4];
    uint16 destport;

    switch(getSn_SSR(s))
    {
	case SOCK_UDP:
	    if ((len = getSn_RX_RSR(s)) > 0)
	    {
		len = recvfrom(s, buf, len, destip, &destport);

		if (len != sendto(s, buf, len, destip, destport))
		{
		    printf("%d : Sendto Fail.len=%d,", s, len);
		    printf("%d.%d.%d.%d:%d\r\n", destip[0], destip[1], destip[2], destip[3], destport);
		}
	    }
	    
	    break;
	case SOCK_CLOSED:
	    close(s);
	    socket(s, Sn_MR_UDP, port, mode);
	    
	    break;
	default:
	    break;
    }
}

int main()
{
    unsigned int secs;
    uint8 buffer[MAX_DNS_LENGTH << 2];

    setup();/*

    while (1)
    {
	loopback(0, 53, buffer, 0);
    }
    */
    secs = getCurrentTime(-6);

    printf("%d\n", secs);

    return 0;
}

void setup()
{
    uint8 txMemConf = 64, rxMemConf = 64;
    
    /* set up the module in memory */
    EBI_CSR2 = 0x00000000;
    EBI_CSR2 |= EBI_CSEN;
    EBI_CSR2 |= EBI_BAT_BYTE_SELECT;
    EBI_CSR2 |= EBI_TDF_1;
    EBI_CSR2 |= EBI_PAGES_1M;
    EBI_CSR2 |= EBI_WSE;
    EBI_CSR2 |= EBI_NWS_8;
    EBI_CSR2 |= EBI_DBW_16;
    EBI_CSR2 |= __DEF_IINCHIP_MAP_BASE__;

    /* do the module's setup */
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
}

uint8 first(uint16 val)
{
    return (val & 0xFF00) >> 8;
}

uint8 second(uint16 val)
{
    return val & 0x00FF;
}

uint16 endianSwap16(uint16 x)
{
    return (x << 8) | (x >> 8);
}

uint32 endianSwap32(uint32 x)
{
    return (x >> 24) |
	   ((x << 8) & 0x00FF0000) |
	   ((x >> 8) & 0x0000FF00) |
	   (x << 24);
}

unsigned int getCurrentTime(int tzShift)
{
    uint8 ntpbuffer[MAX_NTP_LENGTH], server[4];
    uint32 len;
    int secs = 0;

    memset(ntpbuffer, 0, MAX_NTP_LENGTH);

    getIP(NTP_SERVER_DNS_NAME, server);
    if (server[0] == 0)
	memmove(server, NTP_SERVER, 4);
    
    socket(0, Sn_MR_UDP, NTP_CLIENT_PORT, 0);

    ntpbuffer[0] = 0x1B;

    sendto(0, ntpbuffer, MAX_NTP_LENGTH, LAPTOP, NTP_PORT);

    while ((len = getSn_RX_RSR(0)) > 0)
    {
	uint8 destip[4];
	uint16 destport;
	
	len = recvfrom(0, ntpbuffer, len, destip, &destport);
    }

    close(0);

    secs = endianSwap32(ntpbuffer[40]);

    return secs + tzShift * 60 * 60;
}

void getIP(char* dnsName, uint8 *IP)
{
    uint8 dnsbuffer[MAX_DNS_LENGTH], i = 0;
    uint16 *dnsbuffer16 = (uint16*)dnsbuffer;
    uint32 len;

    memset(dnsbuffer, 0, MAX_DNS_LENGTH);
    
    socket(0, Sn_MR_UDP, DNS_CLIENT_PORT, 0);

    dnsbuffer16[i++] = endianSwap16(DNS_HEADER_ID);

    dnsbuffer16[i++] = endianSwap16(DNS_HEADER_QR + DNS_HEADER_OPCODE + DNS_HEADER_AA + DNS_HEADER_TC + DNS_HEADER_RD + DNS_HEADER_RA + DNS_HEADER_Z + DNS_HEADER_RCODE);

    dnsbuffer16[i++] = endianSwap16(DNS_HEADER_QDCOUNT);

    dnsbuffer16[i++] = endianSwap16(DNS_HEADER_ANCOUNT);
    
    dnsbuffer16[i++] = endianSwap16(DNS_HEADER_NSCOUNT);

    dnsbuffer16[i++] = endianSwap16(DNS_HEADER_ARCOUNT);

    strcpy(&(dnsbuffer[i <<= 1]), dnsName);
    i += strlen(dnsName) + 1;

    dnsbuffer[i++] = first(DNS_QUERY_QTYPE);
    dnsbuffer[i++] = second(DNS_QUERY_QTYPE);

    dnsbuffer[i++] = first(DNS_QUERY_QCLASS);
    dnsbuffer[i++] = second(DNS_QUERY_QCLASS);

    /* TODO: set to GW */
    sendto(0, dnsbuffer, i, LAPTOP, DNS_PORT);

    while ((len = getSn_RX_RSR(0)) > 0)
    {
	uint8 destip[4];
	uint16 destport;
	
	len = recvfrom(0, dnsbuffer, len, destip, &destport);
    }

    /* TODO: parse response, get IP */

    close(0);

    /* TODO: return IP */
}

Weather getWeather()
{
    Weather weather;
    weather.low = 0xFF;
    weather.high = 0xFF;

    /* TODO: get weather */

    return weather;
}
