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

#define NTP_SERVER_DNS_NAME "\4time\3srv8ualberta\2ca" /* string literal automatically has \0 at the end */
#define GOOGLE_DNS_NAME "\3www\6google\3com"

#define NTP_SERVER_DNS_IP_OFFSET 0x46
#define GOOGLE_DNS_IP_OFFSET 0x40
#define ECE_DNS_IP_OFFSET 0x43

#define MAX_NTP_LENGTH 48
#define NTP_PORT 123
#define NTP_CLIENT_PORT (NTP_PORT << 9)

#define MAX_HTTP_LENGTH (20 << 10)
#define HTTP_PORT 80
#define HTTP_CLIENT_PORT (HTTP_PORT << 9)
#define HTTP_HEADER_PRE_EMAIL "GET /calendar/feeds/"
#define HTTP_HEADER_POST_EMAIL "/private-"
#define HTTP_HEADER_POST_COOKIE "/basic HTTP/1.1\r\nHOST: www.google.com\r\n\r\n"

#define GMAIL_EMAIL "ticktock490%40gmail.com"
#define GMAIL_MAGIC_COOKIE "cfbbae074cdf44d7eab836cee95940ad"

#define ALARM_ACTIVE_STRING "Alarm Active"

void dns(char *dnsname, int ipoffset, uint8 *servip);
int gcal(uint8 *servip, char *gmail, char *password, char *databuffer); /* for now, password is magic cookie */
unsigned int ntp(uint8 *servip);
   
static uint8 ip[4] = {192, 168, 0, 101};                   // for setting SIP register
static uint8 gw[4] = {192, 168, 0, 1};                     // for setting GAR register
static uint8 sn[4] = {255, 255, 255, 0};                     // for setting SUBR register
static uint8 mac[6] = {0x00, 0x08, 0xDC, 0x00, 111, 200};      // for setting SHAR register

int main()
{
    uint8 tx_mem_conf[8] = {8, 8, 8, 8, 8, 8, 8, 8};          // for setting TMSR regsiter
    uint8 rx_mem_conf[8] = {8, 8, 8, 8, 8, 8, 8, 8};          // for setting RMSR regsiter
    char ntpdnsfixed[22]; /* the \8 doesn't register properly, so need to fix it */
    uint8 defaultntp[4] = {129, 128, 5, 210};
    uint8 defaultgoogle[4] = {74, 125, 53, 103};
    uint8 servip[4];
    unsigned int secs = 0;
    char gdata[MAX_HTTP_LENGTH];
    int gdatasize = 0;
    char *sptr;

    memmove(ntpdnsfixed, NTP_SERVER_DNS_NAME, 22); ntpdnsfixed[9] = 8;
    memset(servip, 0, 4);
    memset(gdata, 0, MAX_HTTP_LENGTH);

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

    dns(ntpdnsfixed, NTP_SERVER_DNS_IP_OFFSET, servip);

    printf("Returned IP: %d.%d.%d.%d\n", servip[0], servip[1], servip[2], servip[3]);    

    if (servip[0] == 0)
	memmove(servip, defaultntp, 4);

    secs = ntp(servip);

    printf("UTC time in seconds: %u\n", secs);

    dns(GOOGLE_DNS_NAME, GOOGLE_DNS_IP_OFFSET, servip);

    printf("Returned IP: %d.%d.%d.%d\n", servip[0], servip[1], servip[2], servip[3]);

    if (servip[0] == 0)
	memmove(servip, defaultgoogle, 4);

    gdatasize = gcal(servip, GMAIL_EMAIL, GMAIL_MAGIC_COOKIE, gdata);

    if (gdatasize < 0)
	printf("Got too many bytes from Google!\n");
    else
    {
	printf("Got %d bytes from Google\n", gdatasize);
    
	sptr = strstr(gdata, ALARM_ACTIVE_STRING);
    
	while (sptr != NULL)
	{
	    int month = 0, day = 0, year = 0, starth = 0, endh = 0, startm = 0, endm = 0;
	    char *monthptr, *sptr2;
	    
	    sptr = strstr(sptr, "When: ");

	    monthptr = sptr + 10;

	    if (strncmp(monthptr, "Jan", 3) == 0)
		month = 1;
	    else if (strncmp(monthptr, "Feb", 3) == 0)
		month = 2;
	    else if (strncmp(monthptr, "Mar", 3) == 0)
		month = 3;
	    else if (strncmp(monthptr, "Apr", 3) == 0)
		month = 4;
	    else if (strncmp(monthptr, "May", 3) == 0)
		month = 5;
	    else if (strncmp(monthptr, "Jun", 3) == 0)
		month = 6;
	    else if (strncmp(monthptr, "Jul", 3) == 0)
		month = 7;
	    else if (strncmp(monthptr, "Aug", 3) == 0)
		month = 8;
	    else if (strncmp(monthptr, "Sep", 3) == 0)
		month = 9;
	    else if (strncmp(monthptr, "Oct", 3) == 0)
		month = 10;
	    else if (strncmp(monthptr, "Nov", 3) == 0)
		month = 11;
	    else if (strncmp(monthptr, "Dec", 3) == 0)
		month = 12;
	    
	    day = atoi(sptr + 14);

	    sptr = strchr(sptr, ',');

	    year = atoi(sptr + 2);
	    starth = atoi(sptr + 7);

	    sptr2 = strchr(sptr, ':');
	    if (sptr2 != NULL && (sptr2 < strstr(sptr, "to")))
		startm = atoi(sptr2 + 1);

	    sptr = strstr(sptr, "to");

	    endh = atoi(sptr + 3);

	    sptr2 = strchr(sptr, ':');
	    if (sptr2 != NULL)
		endm = atoi(sptr2 + 1);

	    if (strncmp(sptr - 3, "pm", 2) == 0)
		starth += 12;

	    sptr = strstr(sptr, "&amp;nbsp;");

	    if (strncmp(sptr - 2, "pm", 2) == 0)
		endh += 12;

	    if (starth % 12 == 0)
		starth -= 12;
	    if (endh % 12 == 0)
		endh -= 12;

	    sptr = strstr(sptr, ALARM_ACTIVE_STRING);

	    printf("Month: %d\n", month);
	    printf("Day: %d\n", day);
	    printf("Year: %d\n", year);
	    printf("Starth: %d\n", starth);
	    printf("Startm: %d\n", startm);
	    printf("Endh: %d\n", endh);
	    printf("Endm: %d\n", endm);

	    /* TODO: save */
	}
    }

    return 0;
}

void dns(char *dnsname, int ipoffset, uint8 *servip)
{
    uint8 i = 0, dnsbuffer[MAX_DNS_LENGTH];
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
}

int gcal(uint8 *servip, char *gmail, char *password, char *databuffer)
{
    uint32 len;
    int i = 0, count = 0;

    close(1);
    socket(1, Sn_MR_TCP, HTTP_CLIENT_PORT, 0);
    connect(1, servip, HTTP_PORT);
	    
    if (getSn_IR(1) & Sn_IR_CON)
	setSn_IR(1, Sn_IR_CON);

    memset(databuffer, 0, MAX_HTTP_LENGTH);

    strcpy(databuffer, HTTP_HEADER_PRE_EMAIL);
    i += strlen(HTTP_HEADER_PRE_EMAIL);

    strcpy(databuffer + i, gmail);
    i += strlen(gmail);

    strcpy(databuffer + i, HTTP_HEADER_POST_EMAIL);
    i += strlen(HTTP_HEADER_POST_EMAIL);

    strcpy(databuffer + i, password);
    i += strlen(password);

    strcpy(databuffer + i, HTTP_HEADER_POST_COOKIE);
    i += strlen(HTTP_HEADER_POST_COOKIE);

    wait_10ms(10);

    send(1, databuffer, i);

    i = 0;
    while (count < 10 && i >= 0)
    {
	if ((len = getSn_RX_RSR(1)) > 0)
	{
	    if ((i + len) > MAX_HTTP_LENGTH)
		i = -1;
	    else
		i += recv(1, databuffer + i, len);
	}
	else
	    ++count;

	wait_10ms(10);
    }
    
    disconnect(1);
    close(1);

    return i;
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
