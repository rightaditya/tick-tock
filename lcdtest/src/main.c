//*----------------------------------------------------------------------------
//* TICK-TOCK Project
//* This program provides touch screen LCD control functions
//*----------------------------------------------------------------------------

#ifdef SEMIHOSTING
#include <stdio.h>
#endif

#include "main.h"

//Libraries
#include "periph/usart/lib_usart.h"
#include "periph/rtc/lib_rtc.h"
#include "periph/rtc/time_rtc.h
#include "parts/m55800/lib_m55800.h"

//Function prototypes
void LCD_init (void);
void setColor(int color);
void clearScreen();

//Constant definitions
#define LCD_BAUD	115200
#define TIMEOUT		50
#define MCKI 		32000000

//Global variables
int currentX, currentY, lightStatus;

//Functions

//*----------------------------------------------------------------------------   
//* Function Name       : bcd_to_int   
//* Object              : Translate bcd to unsigned int   
//* Input Parameters    : <bcd> BCD value   
//* Output Parameters   : integer value   
//*----------------------------------------------------------------------------   
u_int bcd_to_int( u_char bcd)   
{   
     return ( (bcd & 0x0F) + (((bcd & 0xF0)>>4) *10));   
}

//*----------------------------------------------------------------------------   
//* Function Name       : int_to_bcd
//* Object              : Translate int to bcd   
//* Input Parameters    : <value> in int   
//* Output Parameters   : <bcd> BCD value   
//*----------------------------------------------------------------------------   
u_char int_to_bcd (u_int value)   
{   
    char tmp[3];   
   
    sprintf(tmp,"%02d",value);   
    return ((tmp[1] & 0x0F) | ((tmp[0] & 0x0F)<<4));   
}

//*----------------------------------------------------------------------------   
//* Function Name       : RTC_init
//* Object              : Open the rtc   
//* Input Parameters    : none
//* Output Parameters   : none   
//*----------------------------------------------------------------------------   
void RTC_init( void )   
{   
    //* -- Open Real Time Clock   
    at91_rtc_open(&RTC_DESC);   
}

//*----------------------------------------------------------------------------   
//* Function Name       : RTC_close
//* Object              : Close the RTC   
//* Input Parameters    : none
//* Output Parameters   : none   
//*----------------------------------------------------------------------------   
void RTC_close( void )   
{   
    //* -- Close Real Time Clock
    at91_rtc_close(&RTC_DESC);   
}

//*----------------------------------------------------------------------------   
//* Function Name       : RTC_read
//* Object              : Read rtc value   
//* Input Parameters    : none
//* Output Parameters   : update descriptor   
//*----------------------------------------------------------------------------   
void RTC_read( void )   
{   
    u_int time;   
    // get time value   
    time = at91_rtc_get_time (&RTC_DESC);
    printf("second is: %d\n", bcd_to_int(time & RTC_MASQ_SEC));
    /*
    RTC_pt->time_bcd.sec = time & RTC_MASQ_SEC ;   
    RTC_pt->time_bcd.min = (time & RTC_MASQ_MIN)>>RTC_MIN ;   
    RTC_pt->time_bcd.hour = (time & RTC_MASQ_HOUR)>>RTC_HOUR ;   
    RTC_pt->time_bcd.ampm = (time & RTC_MASQ_AMPM)>>RTC_AMPM ;   
    // get calendar value   
    time = at91_rtc_get_calendar (RTC_pt->rtc_desc);   
    RTC_pt->time_bcd.day = (time & RTC_MASQ_DAY)>>RTC_DAY ;   
    RTC_pt->time_bcd.month = (time & RTC_MASQ_MONTH)>>RTC_MONTH ;   
    RTC_pt->time_bcd.year = (time & RTC_MASQ_YEAR)>>RTC_YEAR ;   
    RTC_pt->time_bcd.cent = (time & RTC_MASQ_CENT)>>RTC_CENT;   
    RTC_pt->time_bcd.date = (time & RTC_MASQ_DATE)>>RTC_DATE ;   
    // translate bcd to int   
    RTC_pt->time_int.sec = at91_bcd_int( RTC_pt->time_bcd.sec);   
    RTC_pt->time_int.min = at91_bcd_int( RTC_pt->time_bcd.min);   
    RTC_pt->time_int.hour = at91_bcd_int( RTC_pt->time_bcd.hour);   
    RTC_pt->time_int.ampm = at91_bcd_int( RTC_pt->time_bcd.ampm);   
    RTC_pt->time_int.day = at91_bcd_int( RTC_pt->time_bcd.day);   
    RTC_pt->time_int.month = at91_bcd_int( RTC_pt->time_bcd.month);   
    RTC_pt->time_int.year = at91_bcd_int( RTC_pt->time_bcd.year);   
    RTC_pt->time_int.cent = at91_bcd_int( RTC_pt->time_bcd.cent);   
    RTC_pt->time_int.date = at91_bcd_int( RTC_pt->time_bcd.date);
    */
}

//*----------------------------------------------------------------------------   
//* Function Name       : at91_time_rtc_write_bcd   
//* Object              : Read rtc value   
//* Input Parameters    : <rtc_pt> time rtc descriptor   
//* Output Parameters   : update descriptor   
//*----------------------------------------------------------------------------   
void at91_time_rtc_write_bcd(TimeDescRtc *RTC_pt)   
{   
    if (at91_time_rtc_24mode(RTC_pt)==RTC_24_HRMOD)   
    {   
        //* set ampm at 0   
        RTC_pt->time_bcd.ampm = 0;   
    }   
    // set time   
    at91_rtc_set_time (RTC_pt->rtc_desc,RTC_pt->time_bcd.sec, RTC_pt->time_bcd.min, RTC_pt->time_bcd.hour, RTC_pt->time_bcd.ampm );   
   
    // set date   
    at91_rtc_set_calendar (RTC_pt->rtc_desc, RTC_pt->time_bcd.cent, RTC_pt->time_bcd.year, RTC_pt->time_bcd.month, RTC_pt->time_bcd.day, RTC_pt->time_bcd.date );   
}   

//*----------------------------------------------------------------------------
//* Function Name       : LCD_init
//* Object              : LCD USART initialization 
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------
void LCD_init ( void )
{
    u_short cd_baud; 
    cd_baud=MCKI/(LCD_BAUD*16);
    at91_usart_open(&USART1_DESC, US_ASYNC_MODE, cd_baud, 0);

    setColor(0xFF);
    clearScreen();
}
    
//set the entire screen to the current color
void clearScreen()
{
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0x21); //clear screen
}

//set current color to color, use a two digit hex number
//format is BBGGGRRR in binary
void setColor(int color)
{
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0x24); //set color
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, color); //to green
}

//draw a box at current location
void drawBox(int width, int height)
{
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0x42); //draw box
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, currentX + width - 1); //width
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, currentY + height - 1); //height
}

//draw a box at current location filled with current color
void fillBox(int width, int height)
{
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0x43); //draw box
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, currentX + width - 1); //width
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, currentY + height - 1); //height
}

//set the current location
void setLocation(int x, int y)
{
    currentX = x;
    currentY = y;
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0x25);
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, x);
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, y);
}

//select the font to be used
void selectFont(int fontNum)
{
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0x2B);
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, fontNum);
}

//print a string
void printString(char* str)
{
/*
    int charsLeft = strLen;
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0x2D);
    while(charsLeft != 0)
    {
	while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
	at91_usart_write (&USART1_DESC, str[strLen-charsLeft]);
	charsLeft--;
    }
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0);
    */


    u_int   i;
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0x2D);
    for (i=0; str[i] !=0; i++)   
    {   
        while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0)  {}   
        at91_usart_write(&USART1_DESC, str[i]);   
    }
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0);
}

//draw a line to specified location
void drawLine(int x, int y)
{
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0x28); //draw line
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, x);
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, y);
}

//turn on the LCD light
void lightOn()
{
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0x22);
}

//turn off the LCD light
void lightOff()
{
    while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) == 0) {}
    at91_usart_write (&USART1_DESC, 0x23);
}

//read a byte from the LCD
int LCD_read(u_int* value)
{
    u_int status;   
    status = ((at91_usart_get_status(&USART1_DESC) & US_RXRDY) == 0);   
    if (!status)   
    {
	at91_usart_read (&USART1_DESC, value);
    }
    return !status;   
}

int main()
{
    /*
    //this variable stores data received from the LCD
    u_int* dataReceived;
    
    // Init Usart
    LCD_init();
    selectFont(4);
    setColor(0x07); //red
    setLocation(35, 10);
    printString("T I C K - T O C K");
    setColor(0x38); //green
    setLocation(50, 60);
    printString("1 1 : 2 5 A M");
    setColor(0xC0); //blue
    selectFont(2);
    setLocation(50, 45);
    printString("Wednesday, March 31, 2009");
    setLocation(50, 90);
    printString("High: 27C    Low: 15C");
    setLocation(50, 105);
    printString("Next Alarm: 9:30AM, April 1");

    //draw bottom nav bar
    setColor(0xC0); //blue
    setLocation(0, 130);
    drawBox(240, 30);
    setLocation(60, 130);
    drawLine(60, 159);
    setLocation(180, 130);
    drawLine(180, 159);

    //button text
    setLocation(8, 138);
    printString("Light On");
    setLocation(198, 138);
    printString("Sync");

    //the main loop
    while(1)
    {
	//toggle the LCD light when the light button is pressed
	if(LCD_read(dataReceived) && *dataReceived == 80)
	{
	    //wait for pen up byte
	    setColor(0xC0);
	    setLocation(1, 131);
	    fillBox(59, 28);
	    while (LCD_read(dataReceived) && *dataReceived != 255) {}
	    setColor(0xFF);
    	    setLocation(1, 131);
	    fillBox(59, 28);
	    //toggle light and button text
	    if(lightStatus == 0)
	    {
		lightOn();
		lightStatus = 1;
		setColor(0xC0); //blue
		setLocation(8, 138);
		printString("Light Off");
	    }
	    else
	    {
		lightOff();
		lightStatus = 0;
		setColor(0xC0); //blue
		setLocation(8, 138);
		printString("Light On");
	    }
	}

	
    } //while(1)
    */
    
    RTC_init();
    while(1){
	RTC_read();
    }
    //RTC_close();
    
}

