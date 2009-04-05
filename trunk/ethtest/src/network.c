#include <string.h>

#include "parts/m55800/reg_m55800.h"

#include "types.h"
#include "socket.h"

#include "main.h"

int numberOfAlarms;

void dns(char *dnsname, int ipoffset, uint8 *servip);
int gcal(uint8 *servip, char *bigbuffer); /* for now, password is magic cookie */
unsigned int ntp(uint8 *servip);
   
static uint8 ip[4] = {192, 168, 0, 101};                   // for setting SIP register
static uint8 gw[4] = {192, 168, 0, 1};                     // for setting GAR register
static uint8 sn[4] = {255, 255, 255, 0};                     // for setting SUBR register
static uint8 mac[6] = {0x00, 0x08, 0xDC, 0x00, 111, 200};      // for setting SHAR register

unsigned int netSync()
{
    unsigned int UTCsecs = 0;
    uint8 tx_mem_conf[8] = {8, 8, 8, 8, 8, 8, 8, 8};          // for setting TMSR regsiter
    uint8 rx_mem_conf[8] = {8, 8, 8, 8, 8, 8, 8, 8};          // for setting RMSR regsiter
    char ntpdnsfixed[22]; /* the \8 doesn't register properly, so need to fix it */
    uint8 defaultntp[4] = {129, 128, 5, 210};
    uint8 defaultgoogle[4] = {74, 125, 53, 103};
    uint8 servip[4];
    char *bigbuffer = (char *)NVR_BUFFER_AREA;
    int gdatasize = 0;
    char *sptr;

    memmove(ntpdnsfixed, NTP_SERVER_DNS_NAME, 22); ntpdnsfixed[9] = 8;
    memset(servip, 0, 4);

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

    /* set up the NVRAM on the EBI */
    EBI_CSR3 = 0x00000000;
    EBI_CSR3 |= EBI_CSEN;
    EBI_CSR3 |= EBI_BAT_BYTE_WRITE;
    EBI_CSR3 |= EBI_TDF_1;
    EBI_CSR3 |= EBI_PAGES_4M;
    EBI_CSR3 |= EBI_WSE;
    EBI_CSR3 |= EBI_NWS_5;
    EBI_CSR3 |= EBI_DBW_8;
    EBI_CSR3 |= NVR_BASE;

    dns(ntpdnsfixed, NTP_SERVER_DNS_IP_OFFSET, servip);

    printf("Returned IP: %d.%d.%d.%d\n", servip[0], servip[1], servip[2], servip[3]);    

    if (servip[0] == 0)
	memmove(servip, defaultntp, 4);

    UTCsecs = ntp(servip);

    printf("UTC time in seconds: %u\n", UTCsecs);

    dns(GOOGLE_DNS_NAME, GOOGLE_DNS_IP_OFFSET, servip);

    printf("Returned IP: %d.%d.%d.%d\n", servip[0], servip[1], servip[2], servip[3]);

    if (servip[0] == 0)
	memmove(servip, defaultgoogle, 4);

    gdatasize = gcal(servip, bigbuffer);

    if (gdatasize < 0)
	printf("Got too many bytes from Google!\n");
    else
    {
	Alarm *alarm = (Alarm *)NVR_ALARMS;
	
	printf("Got %d bytes from Google\n", gdatasize);
    
	sptr = strstr(bigbuffer, ALARM_ACTIVE_STRING);
    
	while (sptr != NULL)
	{
	    char *monthptr, *sptr2;

	    memset(alarm, 0, sizeof(Alarm));
	    
	    sptr = strstr(sptr, "When: ");

	    monthptr = sptr + 10;

	    if (strncmp(monthptr, "Jan", 3) == 0)
		alarm->month = 1;
	    else if (strncmp(monthptr, "Feb", 3) == 0)
		alarm->month = 2;
	    else if (strncmp(monthptr, "Mar", 3) == 0)
		alarm->month = 3;
	    else if (strncmp(monthptr, "Apr", 3) == 0)
		alarm->month = 4;
	    else if (strncmp(monthptr, "May", 3) == 0)
		alarm->month = 5;
	    else if (strncmp(monthptr, "Jun", 3) == 0)
		alarm->month = 6;
	    else if (strncmp(monthptr, "Jul", 3) == 0)
		alarm->month = 7;
	    else if (strncmp(monthptr, "Aug", 3) == 0)
		alarm->month = 8;
	    else if (strncmp(monthptr, "Sep", 3) == 0)
		alarm->month = 9;
	    else if (strncmp(monthptr, "Oct", 3) == 0)
		alarm->month = 10;
	    else if (strncmp(monthptr, "Nov", 3) == 0)
		alarm->month = 11;
	    else if (strncmp(monthptr, "Dec", 3) == 0)
		alarm->month = 12;
	    
	    alarm->day = atoi(sptr + 14);

	    sptr = strchr(sptr, ',');

	    alarm->year = atoi(sptr + 2);
	    alarm->starthour = atoi(sptr + 7);

	    sptr2 = strchr(sptr, ':');
	    if (sptr2 != NULL && (sptr2 < strstr(sptr, "to")))
		alarm->startminute = atoi(sptr2 + 1);

	    sptr = strstr(sptr, "to");

	    alarm->endhour = atoi(sptr + 3);

	    sptr2 = strchr(sptr, ':');
	    if (sptr2 != NULL)
		alarm->endminute = atoi(sptr2 + 1);

	    if (strncmp(sptr - 3, "pm", 2) == 0)
		alarm->starthour += 12;

	    sptr = strstr(sptr, "&amp;nbsp;");

	    if (strncmp(sptr - 2, "pm", 2) == 0)
		alarm->endhour += 12;

	    if (alarm->starthour % 12 == 0)
		alarm->starthour -= 12;
	    if (alarm->endhour % 12 == 0)
		alarm->endhour -= 12;

	    sptr = strstr(sptr, ALARM_ACTIVE_STRING);

	    printf("Month: %d\n", alarm->month);
	    printf("Day: %d\n", alarm->day);
	    printf("Year: %d\n", alarm->year);
	    printf("Starth: %d\n", alarm->starthour);
	    printf("Startm: %d\n", alarm->startminute);
	    printf("Endh: %d\n", alarm->endhour);
	    printf("Endm: %d\n", alarm->endminute);

	    ++alarm;
	    ++numberOfAlarms;
	}
    }

    return UTCsecs;
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

    strcpy((char *)(&(dnsbuffer[i <<= 1])), dnsname);
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

int gcal(uint8 *servip, char *bigbuffer)
{
    uint32 len;
    int i = 0, count = 0;
    char databuffer[MAX_HTTP_LENGTH];
    
    memset(databuffer, 0, MAX_HTTP_LENGTH);

    close(1);
    socket(1, Sn_MR_TCP, HTTP_CLIENT_PORT, 0);
    connect(1, servip, HTTP_PORT);
	    
    if (getSn_IR(1) & Sn_IR_CON)
	setSn_IR(1, Sn_IR_CON);

    memset(databuffer, 0, MAX_HTTP_LENGTH);

    strcpy(databuffer, HTTP_HEADER_PRE_USER);
    i += strlen(HTTP_HEADER_PRE_USER);

    strcpy(databuffer + i, (char *)NVR_USERNAME);
    i += strlen((char *)NVR_USERNAME);

    strcpy(databuffer + i, HTTP_HEADER_POST_USER);
    i += strlen(HTTP_HEADER_POST_USER);

    strcpy(databuffer + i, (char *)NVR_PASSWORD);
    i += strlen((char *)NVR_PASSWORD);

    strcpy(databuffer + i, HTTP_HEADER_POST_COOKIE);
    i += strlen(HTTP_HEADER_POST_COOKIE);

    wait_10ms(10);

    send(1, (uint8 *)databuffer, i);

    i = 0;
    while (count < 10 && i >= 0)
    {
	if ((len = getSn_RX_RSR(1)) > 0)
	{
	    len = recv(1, (uint8 *)databuffer, len);

	    if (i + len > NVR_BUFFER_SIZE)
		i = -1;
	    else
	    {
		memmove(bigbuffer + i, databuffer, len);
		i += len;
	    }
	}
	else
	    ++count;

	wait_10ms(10);
    }

    bigbuffer[i] = 0;
    ++i;
    
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
