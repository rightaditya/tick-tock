/******************************************************
 * Aditya Bhargava, Freeman Fan
 *
 * This files contains all constants used in the
 * Tick-Tock program
 *****************************************************/

#ifndef MAIN_H
#define MAIN_H
//Constant definitions

/******************** For the LCD ********************/
//#define DEBUG_MODE	//this is defined when we are running the program from RAM
#define FLASH_SHIFT	16 //number of places to shift touch detection constants when running in flash

//the following are integers reprentating the regions of the LCD screen
//they are used for determining which part of the screen was touched
//the intergers in debug mode and in FLASH mode are different
#ifdef DEBUG_MODE
#define NUM7 		7
#define NUM17		17
#define NUM21		21
#define NUM33		33
#define NUM34		34
#define NUM35		35
#define NUM36		36
#define NUM37		37
#define NUM38		38
#define NUM49		49
#define NUM50		50
#define NUM51		51
#define NUM52		52
#define NUM53		53
#define NUM54		54
#define NUM65		65
#define NUM66		66
#define NUM67		67
#define NUM68		68
#define NUM69		69
#define NUM70		70
#define NUM80		80
#define NUM81		81
#define NUM82		82
#define NUM83		83
#define NUM84		84
#define NUM85		85
#define NUM86		86
#else
#define NUM7		(7 << FLASH_SHIFT)
#define NUM17		(17 << FLASH_SHIFT)
#define NUM21		(21 << FLASH_SHIFT)
#define NUM33		(33 << FLASH_SHIFT)
#define NUM34		(34 << FLASH_SHIFT)
#define NUM35		(35 << FLASH_SHIFT)
#define NUM36		(36 << FLASH_SHIFT)
#define NUM37		(37 << FLASH_SHIFT)
#define NUM38		(38 << FLASH_SHIFT)
#define NUM49		(49 << FLASH_SHIFT)
#define NUM50		(50 << FLASH_SHIFT)
#define NUM51		(51 << FLASH_SHIFT)
#define NUM52		(52 << FLASH_SHIFT)
#define NUM53		(53 << FLASH_SHIFT)
#define NUM54		(54 << FLASH_SHIFT)
#define NUM65		(65 << FLASH_SHIFT)
#define NUM66		(66 << FLASH_SHIFT)
#define NUM67		(67 << FLASH_SHIFT)
#define NUM68		(68 << FLASH_SHIFT)
#define NUM69		(69 << FLASH_SHIFT)
#define NUM70		(70 << FLASH_SHIFT)
#define NUM80		(80 << FLASH_SHIFT)
#define NUM81		(81 << FLASH_SHIFT)
#define NUM82		(82 << FLASH_SHIFT)
#define NUM83		(83 << FLASH_SHIFT)
#define NUM84		(84 << FLASH_SHIFT)
#define NUM85		(85 << FLASH_SHIFT)
#define NUM86		(86 << FLASH_SHIFT)
#endif
//the following are parameters for the LCD
#define LCD_BAUD	115200
#define TIMEOUT		50
#define MCKI 		32000000

//#define SECONDS__20090401	1238544000

//number of days in each month
#define MONTHS_1	31
#define MONTHS_2	28
#define MONTHS_3	31
#define MONTHS_4	30
#define MONTHS_5	31
#define MONTHS_6	30
#define MONTHS_7	31
#define MONTHS_8	31
#define MONTHS_9	30
#define MONTHS_10	31
#define MONTHS_11	30
#define MONTHS_12	31

#define SECSUPTO20090401	3447532800

/******************** For the Weight Sensors ********************/
#define WEIGHT_SAMPLES 10
#define PIOB0_MASK 0x00000001

/******************** For Networking and the NVR ********************/
#define SWAP8(A) (A)
#define SWAP16(A) ((((A << 8) & 0xFF00)) | ((A >> 8) & 0x00FF))
#define SWAP32(A) ((((A << 24) & 0xFF000000)) | (((A << 8) & 0x00FF0000)) | (((A >> 8) & 0x0000FF00)) | (((A >> 24) & 0x000000FF)))

#define FIRSTBYTE(D) ((D & 0xFF00) >> 8)
#define SECONDBYTE(D) (D & 0x00FF)

#define NVR_BASE 0x20000000
#define NVR_SIZE 0x200000
#define NVR_USERNAME (NVR_BASE + 0x20)
#define NVR_PASSWORD (NVR_BASE + 0x40)
#define NVR_TZSHIFT (NVR_BASE + 0x80)
#define NVR_ALARMS (NVR_BASE + 0xA0)
#define NVR_ALARM_BUFFER (NVR_BASE + 0x1000)
#define NVR_BUFFER_AREA (NVR_BASE + (NVR_SIZE >> 1))
#define NVR_BUFFER_SIZE (NVR_BASE + NVR_SIZE - NVR_BUFFER_AREA)

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
//#define ECE_DNS_IP_OFFSET 0x43

#define MAX_NTP_LENGTH 48
#define NTP_PORT 123
#define NTP_CLIENT_PORT (NTP_PORT << 9)

#define MAX_HTTP_LENGTH (20 << 10)
#define HTTP_PORT 80
#define HTTP_CLIENT_PORT (HTTP_PORT << 9)
#define HTTP_HEADER_PRE_USER "GET /calendar/feeds/"
#define HTTP_HEADER_POST_USER "%40gmail.com/private-"
#define HTTP_HEADER_POST_COOKIE "/basic HTTP/1.1\r\nHOST: www.google.com\r\n\r\n"

//#define GMAIL_USER "ticktock490"
//#define GMAIL_MAGIC_COOKIE "cfbbae074cdf44d7eab836cee95940ad"

#define ALARM_ACTIVE_STRING "Alarm Active"

typedef struct
{
    int year, month, day, starthour, startminute, endhour, endminute;
} Alarm;

typedef struct
{
    int date, hour, minute, firstRun;
} timeStruct;

#endif


