//*----------------------------------------------------------------------------
//* TICK-TOCK Project
//* This program provides touch screen LCD control functions
//*----------------------------------------------------------------------------

#ifdef SEMIHOSTING
#include <stdio.h>
#endif

//Libraries
#include "periph/usart/lib_usart.h"
#include "drivers/time_rtc/time_rtc.h"
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
TimeDescRtc *myRTC;

//Functions

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
    myRTC->rtc_desc = &RTC_DESC;
    at91_time_rtc_open(myRTC);
    while(1){
	at91_time_rtc_read(myRTC);
	printf("Second is: %d\n", myRTC->time_bcd.sec);
    }
    //RTC_close();
    
}

