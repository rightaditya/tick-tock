#include <string.h>
#include "parts/m55800/reg_m55800.h"
#include "types.h"
#include "socket.h"

#define SWAP8(A) (A)
#define SWAP16(A) ((((A << 8) & 0xFF00)) | ((A >> 8) & 0x00FF))
#define SWAP32(A) ((((A << 24) & 0xFF000000)) | (((A << 8) & 0x00FF0000)) | (((A >> 8) & 0x0000FF00)) | (((A >> 24) & 0x000000FF)))

#define FIRSTBYTE(D) ((D & 0xFF00) >> 8)
#define SECONDBYTE(D) (D & 0x00FF)

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

#define GOOGLE_DNS_IP_OFFSET 0x40
#define NTP_SERVER_DNS_IP_OFFSET 0x2D

#define MAX_NTP_LENGTH 48
#define NTP_PORT 123
#define NTP_CLIENT_PORT (NTP_PORT << 9)

#define MAX_HTTP_LENGTH (10 << 10)
#define HTTP_PORT 80
#define HTTP_CLIENT_PORT (HTTP_PORT << 9)
#define HTTP_HEADER "GET /calendar/feeds/ticktock490%40gmail.com/private-cfbbae074cdf44d7eab836cee95940ad/basic"

uint8 *dns(char *dnsname, int ipoffset);
char *gdata(uint8 *servip);
unsigned int ntp(uint8 *servip);
   
static uint8 ip[4] = {192, 168, 0, 101};                   // for setting SIP register
static uint8 gw[4] = {192, 168, 0, 1};                     // for setting GAR register
static uint8 sn[4] = {255, 255, 255, 0};                     // for setting SUBR register
static uint8 mac[6] = {0x00, 0x08, 0xDC, 0x00, 111, 200};      // for setting SHAR register
static uint8 LAPTOP[4] = {192, 168, 0, 100};

int main()
{
    uint8 tx_mem_conf[8] = {8, 8, 8, 8, 8, 8, 8, 8};          // for setting TMSR regsiter
    uint8 rx_mem_conf[8] = {8, 8, 8, 8, 8, 8, 8, 8};          // for setting RMSR regsiter
    uint8 *servip, count = 0;
    uint8 databuf[MAX_DNS_LENGTH];
    uint8 defaultgoogle[4] = {74, 125, 155, 104};
    //uint8 defaultntp[4] = {206, 248, 142, 218};
    unsigned int secs = 0;
    char *gdatastuff;

    memset(databuf, 0, MAX_DNS_LENGTH);

    /* Set up the Wiznet on the EBI */
    EBI_CSR2 = 0x00000000;
    EBI_CSR2 |= EBI_CSEN;
    EBI_CSR2 |= EBI_BAT_BYTE_SELECT;
    EBI_CSR2 |= EBI_TDF_1;
    EBI_CSR2 |= EBI_PAGES_1M;
    EBI_CSR2 |= EBI_WSE;
    EBI_CSR2 |= EBI_NWS_8;
    EBI_CSR2 |= EBI_DBW_16;
    EBI_CSR2 |= __DEF_IINCHIP_MAP_BASE__;
   
    /* initiate W5300 */
    iinchip_init();  

    /* allocate internal TX/RX Memory of W5300 */
    if(!sysinit(tx_mem_conf,rx_mem_conf))           
    {
       printf("MEMORY CONFIG ERR.\r\n");
       while(1);
    }

    setMR(getMR()|MR_FS); /* Tell the wiznet to adjust for endianness */
   
    setSHAR(mac); // set source mac address
    setGAR(gw); // set gateway IP address
    setSUBR(sn); // set subnet mask address
    setSIPR(ip); // set source IP address

    /*servip = dns(NTP_SERVER_DNS_NAME, NTP_SERVER_DNS_IP_OFFSET);      

    if (servip == NULL)
	servip = defaultntp;
    else if (servip[0] == 0)
	servip = defaultntp;

    printf("Returned IP: %d.%d.%d.%d\n", servip[0], servip[1], servip[2], servip[3]);*/

    secs = ntp(LAPTOP);

    printf("UTC time in seconds: %u\n", secs);

    servip = dns(GOOGLE_DNS_NAME, GOOGLE_DNS_IP_OFFSET);

    if (servip == NULL)
	servip = defaultgoogle;
    else if (servip[0] == 0)
	servip = defaultgoogle;

    printf("Returned IP: %d.%d.%d.%d\n", servip[0], servip[1], servip[2], servip[3]);

    /* Now we have the time and Google's IP.  GData time. */

    gdatastuff = gdata(servip);

    return 0;
}

uint8 *dns(char *dnsname, int ipoffset)
{
    uint8 i = 0, servip[4], dnsbuffer[MAX_DNS_LENGTH];
    uint16 *dnsbuffer16 = (uint16 *)dnsbuffer;
    uint32 len;
    int received = 0;
    
    close(0);
    socket(0, Sn_MR_UDP, DNS_CLIENT_PORT, 0);

    memset(dnsbuffer, 0, MAX_DNS_LENGTH);

    dnsbuffer16[i++] = SWAP16(DNS_HEADER_ID);
    dnsbuffer16[i++] = SWAP16(DNS_HEADER_QR + DNS_HEADER_OPCODE + DNS_HEADER_AA + DNS_HEADER_TC + DNS_HEADER_RD + DNS_HEADER_RA + DNS_HEADER_Z + DNS_HEADER_RCODE);
    dnsbuffer16[i++] = SWAP16(DNS_HEADER_QDCOUNT);
    dnsbuffer16[i++] = SWAP16(DNS_HEADER_ANCOUNT);
    dnsbuffer16[i++] = SWAP16(DNS_HEADER_NSCOUNT);
    dnsbuffer16[i++] = SWAP16(DNS_HEADER_ARCOUNT);

    strcpy(&(dnsbuffer[i <<= 1]), dnsname);
    i += strlen(dnsname) + 1;

    dnsbuffer[i++] = FIRSTBYTE(DNS_QUERY_QTYPE);
    dnsbuffer[i++] = SECONDBYTE(DNS_QUERY_QTYPE);

    dnsbuffer[i++] = FIRSTBYTE(DNS_QUERY_QCLASS);
    dnsbuffer[i++] = SECONDBYTE(DNS_QUERY_QCLASS);

    sendto(0, dnsbuffer, i, gw, DNS_PORT);

    while (!received)
    {
	if ((len = getSn_RX_RSR(0)) > 0)
	{
	    uint8 destip[4];
	    uint16 destport;

	    len = recvfrom(0, dnsbuffer, len, destip, &destport);
	    received = 1;
	}
    }

    close(0);

    memmove(servip, dnsbuffer + ipoffset, 4);

    return servip;
}

char *gdata(uint8 *servip)
{
    uint8 databuffer[MAX_HTTP_LENGTH];
    uint32 len, i = 0;
    int received = 0;

    close(1);
    socket(1, Sn_MR_TCP, HTTP_CLIENT_PORT, 0);
    connect(1, servip, HTTP_PORT);
	    
    if (getSn_IR(1) & Sn_IR_CON)
	setSn_IR(1, Sn_IR_CON);

    memset(databuffer, 0, MAX_HTTP_LENGTH);

    strcpy(databuffer, HTTP_HEADER);
    i += strlen(HTTP_HEADER);

    send(1, databuffer, i);

    while (!received)
    {
	if ((len = getSn_RX_RSR(1)) > 0)
	{
	    len = recv(1, databuffer, len);
	    received = 1;
	}
    }
    
    disconnect(1);
    close(1);

    return 1;
}

unsigned int ntp(uint8 *servip)
{
    uint8 ntpbuffer[MAX_NTP_LENGTH];
    uint32 len;
    unsigned int *secsptr = NULL;
    int received = 0;

    close(0);
    socket(0, Sn_MR_UDP, NTP_CLIENT_PORT, 0);
    
    memset(ntpbuffer, 0, MAX_NTP_LENGTH);
	    
    ntpbuffer[0] = 0x1B;

    sendto(0, ntpbuffer, MAX_NTP_LENGTH, servip, NTP_PORT);

    while (!received)
    {
	if ((len = getSn_RX_RSR(0)) > 0)
	{
	    uint8 destip[4];
	    uint16 destport;

	    len = recvfrom(0, ntpbuffer, len, destip, &destport);
	    received = 1;
	}
    }

    close(0);

    secsptr = (unsigned int *)&(ntpbuffer[40]);

    return SWAP32(*secsptr);
}
