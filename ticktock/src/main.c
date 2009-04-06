//*----------------------------------------------------------------------------
//* TICK-TOCK Project Main Program
//*
//* Aditya Bhargava, Freeman Fan
//* CMPE 490 Winter Term 2009
//*
//* This program provides a variety of touch screen LCD control functions
//* including drawing shapes and printing strings on the LCD screen.
//* Also included are functions that set and read from the Real Time Clock
//* on the Atmel board and display date, time, weather and alarm information
//* on the screen.
//*
//* Together, these functions are used to create a user interface for the
//* TICK-TOCK device, which is built and constantly updated in a main loop
//* inside the main() function.
//*----------------------------------------------------------------------------

#ifdef SEMIHOSTING
#include <stdio.h>
#endif

//Libraries
#include "drivers/time_rtc/time_rtc.h"
#include "parts/m55800/lib_m55800.h"
#include "periph/usart/lib_usart.h"

#include "main.h"

//Function prototypes
void LCD_init (void);
void setColor(int color);
void clearScreen();

int currentX, currentY, lightStatus; //for drawing on the LCD screen
TimeDescRtc *myRTC; //for storing time and date information
int oldSecond; //flag for indicating time display should be updated
int oldDate; //flag for indicating date display should be updated
//these variables are used to save the date and time of the current and next alarms
Alarm currentAlarm, nextAlarm;
timeStruct syncTime;
int numberOfAlarms;
int alarmBufferPointer; //used to point to the alarm we are working on in the alarm buffer
//this signal is used to control the external speaker
//the speaker is sounding if alarmSounding is true
int alarmSounding;
//this variable is used to indicate which screen we are in
int currentMode; //0=main 1=weather 2=options 3=sync
//this variable is used to indicate where to print a character when entering credentials
int credPosition;
//this variable stores the timezone shift
int *tzShift = (int *)NVR_TZSHIFT;
//this variable stores the temporary credential string (and now also timezone --AB)
char* credString;
//this variable stores the username string
char* usernameString = (char *)NVR_USERNAME;
//this variable stores the password string
char* passwordString = (char *)NVR_PASSWORD;
//this variable stores the next character to be appended to the end of a credential string
char* credAdd;
//this flag is used to indicate whether a char appears above the cursor when entering a credString
int cursorFull;
//this flag indicates whether the alarm output should be turned on
int alarmShouldBeOn;

// alarm lower and upper thresholds for weight
const double LOWER_THRESHOLD = 550, UPPER_THRESHOLD = 2250;

//Functions

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

//this function opens a usart connection to the LCD
void LCD_init ( void )
{
    u_short cd_baud; 
    cd_baud=MCKI/(LCD_BAUD*16);
    at91_usart_open(&USART1_DESC, US_ASYNC_MODE, cd_baud, 0);

    lightOff();
}

//this function closes the usart connection to the LCD
void LCD_close()
{
    at91_usart_close(&USART1_DESC);
}

//this function resets the LCD connection
void LCD_reset()
{
    LCD_close();
    LCD_init();
    if(lightStatus == 1)
	lightOn();
}
    
//set the entire screen to the current color
void clearScreen()
{
    setColor(0xFF);
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

//this function initializes the RTC for keeping track of the date and time
void dateTimeInit()
{
    // allocate memory space for the RTC struct
    myRTC = (TimeDescRtc *)calloc(1, sizeof(TimeDescRtc));
    // bind the RTC descriptor to the corresponding pointer in the RTC struct
    myRTC->rtc_desc = &RTC_DESC;
    // open the RTC
    at91_time_rtc_open(myRTC);
    // set to 24-hour mode
    at91_rtc_set_24(myRTC->rtc_desc);
}

//this function closes the RTC
void dateTimeClose()
{
    //close the RTC
    at91_time_rtc_close(myRTC);
}

//this function reads the current date and time
void dateTimeRead()
{
    at91_time_rtc_read(myRTC);
}

//this function returns the number of hours since the previous sync
int timeSinceSync(int date, int hour, int minute)
{
    int hoursElapsed = 0;
    
    if(date == syncTime.date)
    {
	hoursElapsed = hour - syncTime.hour;
	if(minute < syncTime.minute)
	    hoursElapsed--;
    }
    else if(date > syncTime.date)
    {
	hoursElapsed = (hour + 24) - syncTime.hour;
	if(minute < syncTime.minute)
	    hoursElapsed--;
    }
    return hoursElapsed;
}

//this function buffers eight hours of alarm information on the first run
//on subsequent runs, it ignores all alarm information changes inside the
//eight hour window of a previous sync and update alarm information after
//that sync and before the end of eight hours from now
void updateAlarmBuffer()
{
    int i = 0;
    Alarm *alarm = (Alarm *)NVR_ALARMS;
    Alarm *alarmBuffer = (Alarm *)NVR_ALARM_BUFFER;
    if(syncTime.firstRun == 1) //first run, store the full eight hours
    {
	syncTime.date = myRTC->time_int.date;
	syncTime.hour = myRTC->time_int.hour;
	syncTime.minute = myRTC->time_int.min;
	for(i = 0; i <= numberOfAlarms; ++i)
	{
	    if(timeSinceSync(alarm[i].day, alarm[i].starthour, alarm[i].startminute) < 8)
	    {
		alarmBuffer[alarmBufferPointer].year = alarm[i].year;
		alarmBuffer[alarmBufferPointer].month = alarm[i].month;
		alarmBuffer[alarmBufferPointer].day = alarm[i].day;
		alarmBuffer[alarmBufferPointer].starthour = alarm[i].starthour;
		alarmBuffer[alarmBufferPointer].startminute = alarm[i].startminute;
		alarmBuffer[alarmBufferPointer].endhour = alarm[i].endhour;
		alarmBuffer[alarmBufferPointer].endminute = alarm[i].endminute;
		alarmBufferPointer++;
	    }
	}
	syncTime.firstRun = 0;
    }
    else //not first run, store only new alarm info outside the previous eight hour buffer
    {
	for(i = 0; i <= numberOfAlarms; ++i)
	{
	    if(timeSinceSync(alarm[i].day, alarm[i].starthour, alarm[i].startminute) > 8 &&
		    (timeSinceSync(alarm[i].day, alarm[i].starthour, alarm[i].startminute) -
		    timeSinceSync(myRTC->time_int.date, myRTC->time_int.hour, myRTC->time_int.min)) < 8)
	    {
		alarmBuffer[alarmBufferPointer].year = alarm[i].year;
		alarmBuffer[alarmBufferPointer].month = alarm[i].month;
		alarmBuffer[alarmBufferPointer].day = alarm[i].day;
		alarmBuffer[alarmBufferPointer].starthour = alarm[i].starthour;
		alarmBuffer[alarmBufferPointer].startminute = alarm[i].startminute;
		alarmBuffer[alarmBufferPointer].endhour = alarm[i].endhour;
		alarmBuffer[alarmBufferPointer].endminute = alarm[i].endminute;
		alarmBufferPointer++;
	    }
	}
    }
    //then we clean up the alarms in the buffer that are in the past
}

//this function updates the next alarm info stored by getting the
//closest future alarm in the alarm buffer
void updateNextAlarm()
{
    Alarm *alarm = (Alarm *)NVR_ALARM_BUFFER;
    memset(&nextAlarm, 0, sizeof(Alarm));
    
    if (alarmBufferPointer > 0)
    {
	int i = 0;

	for (i = 0; i < alarmBufferPointer; ++i)
	{
	    int alarmInPast = 0;

	    /* make sure alarm isn't in the past */
	    if (alarm[i].year < (myRTC->time_int.year + myRTC->time_int.cent * 100))
		alarmInPast = 1;
	    else if(alarm[i].year == (myRTC->time_int.year + myRTC->time_int.cent * 100))
	    {
		if (alarm[i].month < myRTC->time_int.month)
		    alarmInPast = 1;
		else if(alarm[i].month == myRTC->time_int.month)
		{
		    if (alarm[i].day < myRTC->time_int.date)
			alarmInPast = 1;
		    else if(alarm[i].day == myRTC->time_int.date)
		    {
			if (alarm[i].starthour < myRTC->time_int.hour)
			    alarmInPast = 1;
			else if(alarm[i].starthour == myRTC->time_int.hour)
			{
			    if (alarm[i].startminute <= myRTC->time_int.min)
				alarmInPast = 1;
			}
		    }
		}
	    }
	    
	    if (!alarmInPast)
	    {
		if (nextAlarm.year == 0)
		    nextAlarm = alarm[i];
		else if (alarm[i].year < nextAlarm.year)
		    nextAlarm = alarm[i];
		else if (alarm[i].year == nextAlarm.year)
		{
		    if (alarm[i].month < nextAlarm.month)
			nextAlarm = alarm[i];
		    else if(alarm[i].month == nextAlarm.month)
		    {
			if (alarm[i].day < nextAlarm.day)
			    nextAlarm = alarm[i];
			else if(alarm[i].day == nextAlarm.day)
			{
			    if (alarm[i].starthour < nextAlarm.starthour)
				nextAlarm = alarm[i];
			    else if(alarm[i].starthour == nextAlarm.starthour)
			    {
				if (alarm[i].startminute < nextAlarm.startminute)
				    nextAlarm = alarm[i];
			    }
			}
		    }
		}
	    }
	}
    }
}

//this function synchronizes time, date and calendar info with Internet source
void sync()
{
    unsigned int UTCsecs;
    unsigned int secsSince20090401;
	
    //these variables are used to save the date and time values read online
    int currentYear, currentMonth, currentDate, currentDay, currentHour, currentMinute, currentSecond;//, currentAMPM;

    //clear the screen first
    clearScreen();

    //draw title text
    selectFont(4);
    setColor(0x07); //red
    setLocation(15, 10);
    printString("SYNCHRONIZING");
    setLocation(74, 25);
    printString(".  .  .  .  .  .");

    //connect to the Internet and retrieve time and alarm information
    UTCsecs = netSync();

    secsSince20090401 = (UTCsecs - SECSUPTO20090401) + 60 * 60 * (*tzShift); // + 60 * 60 * (*tzShift)

    //read time and date values online
    currentYear = (secsSince20090401/86400) / 365 + 2009;
    currentMonth = (secsSince20090401/86400) % 365 + 1;
    if(currentMonth <= 31) currentMonth = 4;
    else if(currentMonth>31 && currentMonth<=59) currentMonth = 5;
    else if(currentMonth>59 && currentMonth<=90) currentMonth = 6;
    else if(currentMonth>90 && currentMonth<=120) currentMonth = 7;
    else if(currentMonth>120 && currentMonth<=151) currentMonth = 8;
    else if(currentMonth>151 && currentMonth<=181) currentMonth = 9;
    else if(currentMonth>181 && currentMonth<=212) currentMonth = 10;
    else if(currentMonth>212 && currentMonth<=243) currentMonth = 11;
    else if(currentMonth>243 && currentMonth<=273) currentMonth = 12;
    else if(currentMonth>273 && currentMonth<=304) currentMonth = 1;
    else if(currentMonth>304 && currentMonth<=334) currentMonth = 2;
    else if(currentMonth>334 && currentMonth<=365) currentMonth = 3;
    currentDate = (secsSince20090401/86400) % 365 + 1;
    if(currentMonth == 5) currentDate -= 31;
    else if(currentMonth == 6) currentDate -= 59;
    else if(currentMonth == 7) currentDate -= 90;
    else if(currentMonth == 8) currentDate -= 120;
    else if(currentMonth == 9) currentDate -= 151;
    else if(currentMonth == 10) currentDate -= 181;
    else if(currentMonth == 11) currentDate -= 212;
    else if(currentMonth == 12) currentDate -= 243;
    else if(currentMonth == 1) currentDate -= 273;
    else if(currentMonth == 2) currentDate -= 304;
    else if(currentMonth == 3) currentDate -= 334;
    currentDay = (secsSince20090401/86400 + 2) % 7 + 1;
    currentHour = (secsSince20090401/3600) % 24;
    currentMinute = (secsSince20090401/60) % 60;
    currentSecond = secsSince20090401 % 60;

    myRTC->time_int.cent = currentYear / 100;
    myRTC->time_int.year = currentYear - myRTC->time_int.cent * 100;
    myRTC->time_int.month = currentMonth;
    myRTC->time_int.date = currentDate;
    myRTC->time_int.day = currentDay;
    myRTC->time_int.hour = currentHour;
    myRTC->time_int.min = currentMinute;
    myRTC->time_int.sec = currentSecond;
    //myRTC->time_int.ampm = currentAMPM;

    at91_time_rtc_write(myRTC);

    //update the eight hour alarm buffer if needed
    updateAlarmBuffer();
    //update the time of the most recent sync
    syncTime.date = currentDate;
    syncTime.hour = currentHour;
    syncTime.minute = currentMinute;

    //update the next alarm
    updateNextAlarm();
}

//this function updates the time zone info stored
void updateTZ(int newTZ)
{
    //these variables are used to save the date and time values
    int currentYear, currentMonth, currentDate, currentDay, currentHour, oldTZ = *tzShift;

    *tzShift = newTZ;
    //clear the screen first
    clearScreen();

    //draw title text
    selectFont(4);
    setColor(0x07); //red
    setLocation(55, 60);
    printString("UPDATING");
    setLocation(74, 75);
    printString(".  .  .  .  .  .");

    currentYear = myRTC->time_int.year;
    currentMonth = myRTC->time_int.month;
    currentDate = myRTC->time_int.date;
    currentDay = myRTC->time_int.day;
    currentHour = myRTC->time_int.hour += *tzShift - oldTZ;

    if(currentHour < 0)
    {
	currentHour += 24;
	currentDay--;
	currentDate--;
    }
    else if(currentHour > 23)
    {
	currentHour -= 24;
	currentDay++;
	currentDate++;
    }

    if(currentDate <= 0)
    {
	currentMonth--;
	if(currentMonth == 0) currentDate = 31;
	else if(currentMonth == 1) currentDate = 31;
	else if(currentMonth == 2) currentDate = 28;
	else if(currentMonth == 3) currentDate = 31;
	else if(currentMonth == 4) currentDate = 30;
	else if(currentMonth == 5) currentDate = 31;
	else if(currentMonth == 6) currentDate = 30;
	else if(currentMonth == 7) currentDate = 31;
	else if(currentMonth == 8) currentDate = 31;
	else if(currentMonth == 9) currentDate = 30;
	else if(currentMonth == 10) currentDate = 31;
	else if(currentMonth == 11) currentDate = 30;
    }
    else if((currentDate > 28 && currentMonth == 2) ||
	    (currentDate > 30 && (currentMonth == 4 || currentMonth == 6 || currentMonth == 9 || currentMonth == 11)) ||
	    currentDate > 31)
    {
	currentDate = 1;
	currentMonth++;
    }
    
    if(currentMonth <= 0)
    {
	currentMonth += 12;
	currentYear--;
    }
    else if(currentMonth >= 13)
    {
	currentMonth -= 12;
	currentYear++;
    }
    currentDay %= 7 + 1;
    
    myRTC->time_int.month = currentMonth;
    myRTC->time_int.date = currentDate;
    myRTC->time_int.day = currentDay;
    myRTC->time_int.hour = currentHour;
    myRTC->time_int.year = currentYear;
    
    at91_time_rtc_write(myRTC);
}

//this function prints the current time on the LCD screen
//by erasing only the part of the screen that needs to be updated
//in order to minimize flickering
void printTime()
{
    //this is used to store the time display string
    char timeString[17];

    //check if a second has passed
    //if not then we don't need to update time display, so return
    if(myRTC->time_int.sec == oldSecond) return;

    //convert 24 hour time into 12 hour AM/PM mode
    if(myRTC->time_int.hour == 12)
	sprintf(timeString, "%02d : %02d : %02d PM", myRTC->time_int.hour, myRTC->time_int.min, myRTC->time_int.sec);
    else if(myRTC->time_int.hour > 12)
	sprintf(timeString, "%02d : %02d : %02d PM", myRTC->time_int.hour - 12, myRTC->time_int.min, myRTC->time_int.sec);
    else if (myRTC->time_int.hour == 0)
	sprintf(timeString, "%02d : %02d : %02d AM", myRTC->time_int.hour + 12, myRTC->time_int.min, myRTC->time_int.sec);
    else
	sprintf(timeString, "%02d : %02d : %02d AM", myRTC->time_int.hour, myRTC->time_int.min, myRTC->time_int.sec);
    
    setColor(0xFF);
    setLocation(50, 60);
    fillBox(190, 35);
    selectFont(4);
    setColor(0x38); //green
    setLocation(40, 70);
    printString(timeString);
    oldSecond = myRTC->time_int.sec; //save the current second
}

//this function prints the current date on the LCD screen
void printDate()
{
    //these are used to store the date and month strings
    char month[4];
    
    //this is used to store the date display string
    char dateString[23];

    //check if a day has passed
    //if not then we don't need to update date display, so return
    if(myRTC->time_int.date == oldDate) return;

    //build the text string for the month
    switch(myRTC->time_int.month)
    {
	case(1):
	    sprintf(month, "Jan");
	    break;
	case(2):
	    sprintf(month, "Feb");
	    break;
	case(3):
	    sprintf(month, "Mar");
	    break;
	case(4):
	    sprintf(month, "Apr");
	    break;
	case(5):
	    sprintf(month, "May");
	    break;
	case(6):
	    sprintf(month, "Jun");
	    break;
	case(7):
	    sprintf(month, "Jul");
	    break;
	case(8):
	    sprintf(month, "Aug");
	    break;
	case(9):
	    sprintf(month, "Sep");
	    break;
	case(10):
	    sprintf(month, "Oct");
	    break;
	case(11):
	    sprintf(month, "Nov");
	    break;
	case(12):
	    sprintf(month, "Dec");
	    break;
	default:
	    break;
    }

    //build the text string for the entire date display line
    switch(myRTC->time_int.day)
    {
	case(1):
	    sprintf(dateString, "Monday, %s %d, %d", month, myRTC->time_int.date, myRTC->time_int.cent * 100 + myRTC->time_int.year);
	    break;
	case(2):
	    sprintf(dateString, "Tuesday, %s %d, %d", month, myRTC->time_int.date, myRTC->time_int.cent * 100 + myRTC->time_int.year);
	    break;
	case(3):
	    sprintf(dateString, "Wednesday, %s %d, %d", month, myRTC->time_int.date, myRTC->time_int.cent * 100 + myRTC->time_int.year);
	    break;
	case(4):
	    sprintf(dateString, "Thursday, %s %d, %d", month, myRTC->time_int.date, myRTC->time_int.cent * 100 + myRTC->time_int.year);
	    break;
	case(5):
	    sprintf(dateString, "Friday, %s %d, %d", month, myRTC->time_int.date, myRTC->time_int.cent * 100 + myRTC->time_int.year);
	    break;
	case(6):
	    sprintf(dateString, "Saturday, %s %d, %d", month, myRTC->time_int.date, myRTC->time_int.cent * 100 + myRTC->time_int.year);
	    break;
	case(7):
	    sprintf(dateString, "Sunday, %s %d, %d", month, myRTC->time_int.date, myRTC->time_int.cent * 100 + myRTC->time_int.year);
	    break;
        default:
	    break;
    }

    setColor(0xFF);
    setLocation(50,45);
    fillBox(190, 14);
    selectFont(2);
    setColor(0xC0); //blue
    setLocation(50, 45);
    printString(dateString);
    oldDate = myRTC->time_int.date; //save the current date
}

//this function prints the next alarm information
void printNextAlarm()
{
    char nextAlarmString[34];
    
    if (nextAlarm.year == 0)
	strcpy(nextAlarmString, "Next alarm: none");
    else
    {
	int alarmHourDisplay;
	int alarmAMPMDisplay;
	
	//these are used to store the month and next alarm strings
	char month[4];
	
	//build the text string for the month
	switch(nextAlarm.month)
	{
	    case(1):
		sprintf(month, "Jan");
		break;
	    case(2):
		sprintf(month, "Feb");
		break;
	    case(3):
		sprintf(month, "Mar");
		break;
	    case(4):
		sprintf(month, "Apr");
		break;
	    case(5):
		sprintf(month, "May");
		break;
	    case(6):
		sprintf(month, "Jun");
		break;
	    case(7):
		sprintf(month, "Jul");
		break;
	    case(8):
		sprintf(month, "Aug");
		break;
	    case(9):
		sprintf(month, "Sep");
		break;
	    case(10):
		sprintf(month, "Oct");
		break;
	    case(11):
		sprintf(month, "Nov");
		break;
	    case(12):
		sprintf(month, "Dec");
		break;
	    default:
		break;
	}
    
	if(nextAlarm.starthour == 0)
	{
	    alarmHourDisplay = 12;
	    alarmAMPMDisplay = 0; //AM
	}
	else if(nextAlarm.starthour >= 1 && nextAlarm.starthour <= 11)
	{
	    alarmHourDisplay = nextAlarm.starthour;
	    alarmAMPMDisplay = 0;
	}
	else if(nextAlarm.starthour == 12)
	{
	    alarmHourDisplay = 12;
	    alarmAMPMDisplay = 1; //PM
	}
	else if(nextAlarm.starthour >= 13 && nextAlarm.starthour <= 23)
	{
	    alarmHourDisplay = nextAlarm.starthour - 12;
	    alarmAMPMDisplay = 1;
	}
    
	sprintf(nextAlarmString, "Next alarm: %02d:%02d %s, %s %d", alarmHourDisplay, nextAlarm.startminute, alarmAMPMDisplay ? "PM" : "AM", month, nextAlarm.day);
    }

    setColor(0xC0); //blue
    selectFont(2);
    setLocation(50, 105);
    printString(nextAlarmString);
}

//this function prints a title text on the top part of the screen
void printTitle(int sel)
{
    setLocation(30, 5);
    setColor(0xFF); //white
    fillBox(200, 40);
    selectFont(4);
    setColor(0x07); //red
    setLocation(35, 10);
	
    if(sel == 0)
    {
	printString("T I C K - T O C K");
    }
    else if(sel == 1)
    {
	printString("W A K E   U P ! !");
    }
}

//this function draws the bottom nav bar
void drawNavBar()
{
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
    setLocation(68, 138);
    printString("          Options");
    setLocation(198, 138);
    printString("Sync");
}

//this displays the main screen
void mainScreen()
{
    //reset saved date and time variables so date and time can be updated
    oldDate = 0;
    oldSecond = 61;
    
    //clear the screen first
    clearScreen();
    
    //print the tick-tock text
    printTitle(0);

    //print the current time
    printTime();
    
    //print the current date
    printDate();
    
    //print next alarm string
    printNextAlarm();

    //draw the bottom nav bar
    drawNavBar();
}

//check if the alarm should be on
void doAlarm()
{
    if (currentAlarm.year == 0) /* if no current alarm is set */
    {
	if (nextAlarm.year != 0) /* if there is a next alarm */
	{
	    if ((nextAlarm.year % 100 == myRTC->time_int.year) &&
		(nextAlarm.month == myRTC->time_int.month) &&
		(nextAlarm.day == myRTC->time_int.date) &&
		(nextAlarm.starthour == myRTC->time_int.hour) &&
		(nextAlarm.startminute == myRTC->time_int.min))
	    {
		currentAlarm = nextAlarm;

		if (alarmActive(LOWER_THRESHOLD, UPPER_THRESHOLD))
		    alarmShouldBeOn = 1;

		updateNextAlarm(); mainScreen();
	    }
	}
    }
    else
    {
	/* check if alarm period is over */
	if ((currentAlarm.endhour <= myRTC->time_int.hour) && (currentAlarm.endminute <= myRTC->time_int.min))
	{
	    memset(&currentAlarm, 0, sizeof(Alarm));

	    alarmShouldBeOn = 0; /* since alarmShouldBeOn defaults to zero, this is just to be clear */
	}
	else
	{
	    if (alarmActive(LOWER_THRESHOLD, UPPER_THRESHOLD))
		alarmShouldBeOn = 1;
	}
    }

    if (alarmShouldBeOn && !alarmSounding)
    {
	beepOn();
	printTitle(1);
    }
    else if (!alarmShouldBeOn && alarmSounding)
    {
	beepOff();
	printTitle(0);
    }
}

//display the options screen and allow the user the change program options
void showOptions()
{
    clearScreen();

    //draw title text
    selectFont(4);
    setColor(0x38); //green
    setLocation(35, 1);
    printString("   O P T I O N S");
    
    //draw bottom nav bar
    selectFont(2);
    setColor(0xC0); //blue
    setLocation(60, 27);
    drawBox(120, 107);
    setLocation(60, 54);
    drawLine(179, 54);
    setLocation(60, 80);
    drawLine(179, 80);
    setLocation(60, 107);
    drawLine(179, 107);

    //button text
    setLocation(68, 33);
    printString("Google User Name");
    setLocation(68, 60);
    printString("Magic Cookie");
    setLocation(68, 86);
    printString("Enter Time Zone");
    setLocation(68, 113);
    printString("Back to Clock");
}

//display the weather screen and show today and tomorrow's weather
void showWeather()
{
    clearScreen();

    //draw title text
    selectFont(4);
    setColor(0x38); //green
    setLocation(46, 1);
    printString("W E A T H E R");
    
    //draw bottom nav bar
    selectFont(2);
    setColor(0xC0); //blue
    setLocation(0, 133);
    drawBox(240, 27);
    
    //button text
    setLocation(85, 139);
    printString("Back to Clock");
}

//print a new username or password char
void credPrint(char* credChar)
{
    setColor(0xFF); //white
    setLocation((38 + credPosition*10), 34);
    fillBox(10, 12);
    setLocation((38 + credPosition*10), 34);
    setColor(0x38); //green
    printString(credChar);
    credAdd = credChar;
    cursorFull = 1;
}

//store the current char to the credString and move the cred cursor
void credNext()
{
    if(cursorFull == 0 || credPosition >= 16) return;
    
    //remove old cursor
    setColor(0xFF);
    setLocation((38 + credPosition*10), 46);
    drawLine((38 + credPosition*10) + 8, 46);

    //save newly typed char to the credString
    sprintf(credString, "%s%s", credString, credAdd);
    credPosition++;
    
    //draw cursor
    setColor(0xC0); //blue
    setLocation((38 + credPosition*10), 46);
    drawLine((38 + credPosition*10) + 8, 46);

    cursorFull = 0;

    credAdd = NULL;
}

//remove a username or password char
void credRemove()
{
    //used to store a copy of the credString
    char credStringCopy[20];

    int count; //counter
    
    //used to store the length of the credString
    int credLength;

    //clear the currently stored credChar
    credAdd = NULL;

    //if a char is above the cursor, only erase it from screen because
    //it has not yet been saved to credString
    if(cursorFull == 1)
    {
	setColor(0xFF); //white
	setLocation((38 + (credPosition)*10), 34);
	fillBox(10, 12);
	cursorFull = 0;
	return;
    }

    //otherwise remove the char from screen, move cursor backward and update credString
    if(credPosition > 0)
    {
	//remove old cursor and char
	setColor(0xFF); //white
	setLocation((38 + (credPosition)*10), 34);
	fillBox(10, 15);
	credPosition--;

	setColor(0xFF); //white
	setLocation((38 + (credPosition)*10), 34);
	fillBox(10, 15);

	//draw cursor
	setColor(0xC0);
	setLocation((38 + (credPosition)*10), 46);
	drawLine((38 + (credPosition)*10) + 8, 46);

	//remove the last char from credString
	credLength = strlen(credString);
	strcpy(credStringCopy, credString);
	memset(credString, 0, 20*sizeof(char));
	for(count=0; count<credLength-1; count++)
	{
	    sprintf(credString, "%s%c", credString, credStringCopy[count]);
	}
    }
}

//this function draws the cell phone style keyboard
void keyboardDraw(int keyMode)
{
    //draw keyboard frame
    setColor(0xFF);
    setLocation(30,53);
    fillBox(180,107); //clear this area
    setColor(0xC0);
    setLocation(30, 53);
    drawBox(180, 107);
    setLocation(30, 80);
    drawLine(209, 80);
    setLocation(30, 107);
    drawLine(209, 107);
    setLocation(30, 133);
    drawLine(209, 133);
    setLocation(90, 53);
    drawLine(90, 159);
    setLocation(150, 53);
    drawLine(150, 159);

    //draw keyboard text according to keyMode
    if(keyMode == 0)
    {
	setLocation(38,60);
	printString("Number");
	setLocation(98, 60);
	printString("abc");
	setLocation(158, 60);
	printString("def");
	setLocation(38, 87);
	printString("ghi");
	setLocation(98, 87);
	printString("jkl");
	setLocation(158, 87);
	printString("mno");
	setLocation(38, 114);
	printString("pqrs");
	setLocation(98, 114);
	printString("tuv");
	setLocation(158, 114);
	printString("wxyz");
	setLocation(38, 140);
	printString("Caps");
	setLocation(98, 140);
	printString("Next");
	setLocation(158, 140);
	printString("Back");
    }
    else if(keyMode == 1)
    {
	setLocation(38,60);
	printString("Number");
	setLocation(98, 60);
	printString("ABC");
	setLocation(158, 60);
	printString("DEF");
	setLocation(38, 87);
	printString("GHI");
	setLocation(98, 87);
	printString("JKL");
	setLocation(158, 87);
	printString("MNO");
	setLocation(38, 114);
	printString("PQRS");
	setLocation(98, 114);
	printString("TUV");
	setLocation(158, 114);
	printString("WXYZ");
	setLocation(38, 140);
	printString("Caps");
	setLocation(98, 140);
	printString("Next");
	setLocation(158, 140);
	printString("Back");
    }
    else if(keyMode == 2)
    {
	setLocation(38,60);
	printString("1");
	setLocation(98, 60);
	printString("2");
	setLocation(158, 60);
	printString("3");
	setLocation(38, 87);
	printString("4");
	setLocation(98, 87);
	printString("5");
	setLocation(158, 87);
	printString("6");
	setLocation(38, 114);
	printString("7");
	setLocation(98, 114);
	printString("8");
	setLocation(158, 114);
	printString("9");
	setLocation(38, 140);
	printString("Letter");
	setLocation(98, 140);
	printString("0");
	setLocation(158, 140);
	printString("Back");
    }
    else if(keyMode == 3)
    {
	setLocation(38,60);
	printString("1");
	setLocation(98, 60);
	printString("2");
	setLocation(158, 60);
	printString("3");
	setLocation(38, 87);
	printString("4");
	setLocation(98, 87);
	printString("5");
	setLocation(158, 87);
	printString("6");
	setLocation(38, 114);
	printString("7");
	setLocation(98, 114);
	printString("8");
	setLocation(158, 114);
	printString("9");
	setLocation(38, 140);
	printString("-");
	setLocation(98, 140);
	printString("0");
	setLocation(158, 140);
	printString("Back");
    }
    else if(keyMode == 4)
    {
	setLocation(38,60);
	printString("1");
	setLocation(98, 60);
	printString("2");
	setLocation(158, 60);
	printString("3");
	setLocation(38, 87);
	printString("4");
	setLocation(98, 87);
	printString("5");
	setLocation(158, 87);
	printString("6");
	setLocation(38, 114);
	printString("7");
	setLocation(98, 114);
	printString("8");
	setLocation(158, 114);
	printString("9");
	setLocation(38, 140);
	printString("+");
	setLocation(98, 140);
	printString("0");
	setLocation(158, 140);
	printString("Back");
    }
}

//this function is used to print a debugging string
void dprint(char* debug_string)
{
    setColor(0xFF);
    setLocation(0, 0);
    fillBox(240,10);
    setColor(0xC0);
    setLocation(0, 0);
    selectFont(1);
    printString(debug_string);
}

//display the username entry screen and get username input from the user
void credEntry(int credMode)
{
    //this variable is used to determine whether the cursor should move forward on a key press
    //the cursor should move forward when a different button from the previous one is pressed
    int oldID;
    
    //this is a string used for debugging
    //char debug_string[40];
    
    //this flag is used to indicate whether we are typing in
    //upper case letters, lower case letters, or numbers
    //0=lower case 1=upper case 2=numbers
    int keyMode = 0;
    int id = 0;
    //int i = 0;

    //this variable stores data received from the LCD
    u_int* dataReceived;

    //this flag is used to indicate whether the cursor has a char above it
    cursorFull = 0;

    //clear the credString
    memset(credString, 0, 20*sizeof(char));

    //reset input text position
    credPosition = 0;

    clearScreen();

    //draw title text
    selectFont(2);
    setColor(0xC0); //blue
    setLocation(5, 7);
    if(credMode == 0) printString("Enter your Google user name:");
    else if(credMode == 1) printString("Enter your Magic Cookie:");

    //draw enter button
    setLocation(180, 0);
    drawBox(60, 27);
    setLocation(191, 6);
    printString("Submit");

    //draw text field
    setLocation(30, 30);
    drawBox(180, 21);

    //draw cursor
    setLocation(38, 46);
    drawLine(46, 46);

    keyboardDraw(keyMode);

    while(1)
    {
	if(LCD_read(dataReceived))
	{
	    oldID = id;

	    //////////////////////////////DEBUG/////////////////////////////////
	    //sprintf(debug_string, "dataReceived is: %d\n", *dataReceived);
	    //dprint(debug_string);
	    ////////////////////////////////////////////////////////////////////
	    
	    //when a button is pressed, process the command accordingly
	    if(*dataReceived == NUM33 || *dataReceived == NUM34) //row 1 column 1
	    {
		id = 1;

		if (oldID > 0 && id != oldID)
		    credNext();
		
		if(keyMode == 0 || keyMode == 1)
		{
		    keyMode = 2;
		    keyboardDraw(keyMode);
		    id = 0; credNext();
		}
		else if(keyMode == 2)
		{
		    credPrint("1");
		}
	    }
	    else if(*dataReceived == NUM35 || *dataReceived == NUM36) //row 1 column 2
	    {
		id = 2;

		if (oldID > 0 && id != oldID)
		    credNext();
		
		if(keyMode == 0) //text mode
		{
		    if(strcmp(credAdd, "a") == 0) credPrint("b");
		    else if(strcmp(credAdd, "b") == 0) credPrint("c");
		    else credPrint("a");
		}
		else if(keyMode == 1)
		{
		    if(strcmp(credAdd, "A") == 0) credPrint("B");
		    else if(strcmp(credAdd, "B") == 0) credPrint("C");
		    else credPrint("A");
		}
		else if(keyMode == 2)
		{
		    credPrint("2");
		}
	    }
	    else if(*dataReceived == NUM37 || *dataReceived == NUM38) //row 1 column 3
	    {
		id = 3;

		if (oldID > 0 && id != oldID)
		    credNext();
		
		if(keyMode == 0) //text mode
		{
		    if(strcmp(credAdd, "d") == 0) credPrint("e");
		    else if(strcmp(credAdd, "e") == 0) credPrint("f");
		    else credPrint("d");
		}
		else if(keyMode == 1)
		{
		    if(strcmp(credAdd, "D") == 0) credPrint("E");
		    else if(strcmp(credAdd, "E") == 0) credPrint("F");
		    else credPrint("D");
		}
		else if(keyMode == 2)
		{
		    credPrint("3");
		}
	    }
	    else if(*dataReceived == NUM49 || *dataReceived == NUM50) //row 2 column 1
	    {
		id = 4;

		if (oldID > 0 && id != oldID)
		    credNext();
		
		if(keyMode == 0) //text mode
		{
		    if(strcmp(credAdd, "g") == 0) credPrint("h");
		    else if(strcmp(credAdd, "h") == 0) credPrint("i");
		    else credPrint("g");
		}
		else if(keyMode == 1)
		{
		    if(strcmp(credAdd, "G") == 0) credPrint("H");
		    else if(strcmp(credAdd, "G") == 0) credPrint("I");
		    else credPrint("G");
		}
		else if(keyMode == 2)
		{
		    credPrint("4");
		}
	    }
	    else if(*dataReceived == NUM51 || *dataReceived == NUM52) //row 2 column 2
	    {
		id = 5;

		if (oldID > 0 && id != oldID)
		    credNext();
		
		if(keyMode == 0) //text mode
		{
		    if(strcmp(credAdd, "j") == 0) credPrint("k");
		    else if(strcmp(credAdd, "k") == 0) credPrint("l");
		    else credPrint("j");
		}
		else if(keyMode == 1)
		{
		    if(strcmp(credAdd, "J") == 0) credPrint("K");
		    else if(strcmp(credAdd, "K") == 0) credPrint("L");
		    else credPrint("J");
		}
		else if(keyMode == 2)
		{
		    credPrint("5");
		}
	    }
	    else if(*dataReceived == NUM53 || *dataReceived == NUM54) //row 2 column 3
	    {
		id = 6;

		if (oldID > 0 && id != oldID)
		    credNext();
		
		if(keyMode == 0) //text mode
		{
		    if(strcmp(credAdd, "m") == 0) credPrint("n");
		    else if(strcmp(credAdd, "n") == 0) credPrint("o");
		    else credPrint("m");
		}
		else if(keyMode == 1)
		{
		    if(strcmp(credAdd, "M") == 0) credPrint("N");
		    else if(strcmp(credAdd, "N") == 0) credPrint("O");
		    else credPrint("M");
		}
		else if(keyMode == 2)
		{
		    credPrint("6");
		}
	    }
	    else if(*dataReceived == NUM65 || *dataReceived == NUM66) //row 3 column 1
	    {
		id = 7;

		if (oldID > 0 && id != oldID)
		    credNext();
		
		if(keyMode == 0) //text mode
		{
		    if(strcmp(credAdd, "p") == 0) credPrint("q");
		    else if(strcmp(credAdd, "q") == 0) credPrint("r");
		    else if(strcmp(credAdd, "r") == 0) credPrint("s");
		    else credPrint("p");
		}
		else if(keyMode == 1)
		{
		    if(strcmp(credAdd, "P") == 0) credPrint("Q");
		    else if(strcmp(credAdd, "Q") == 0) credPrint("R");
		    else if(strcmp(credAdd, "R") == 0) credPrint("S");
		    else credPrint("P");
		}
		else if(keyMode == 2)
		{
		    credPrint("7");
		}
	    }
	    else if(*dataReceived == NUM67 || *dataReceived == NUM68) //row 3 column 2
	    {
		id = 8;

		if (oldID > 0 && id != oldID)
		    credNext();
		
		if(keyMode == 0) //text mode
		{
		    if(strcmp(credAdd, "t") == 0) credPrint("u");
		    else if(strcmp(credAdd, "u") == 0) credPrint("v");
		    else credPrint("t");
		}
		else if(keyMode == 1)
		{
		    if(strcmp(credAdd, "T") == 0) credPrint("U");
		    else if(strcmp(credAdd, "U") == 0) credPrint("V");
		    else credPrint("T");
		}
		else if(keyMode == 2)
		{
		    credPrint("8");
		}
	    }
	    else if(*dataReceived == NUM69 || *dataReceived == NUM70) //row 3 column 3
	    {
		id = 9;

		if (oldID > 0 && id != oldID)
		    credNext();
		
		if(keyMode == 0) //text mode
		{
		    if(strcmp(credAdd, "w") == 0) credPrint("x");
		    else if(strcmp(credAdd, "x") == 0) credPrint("y");
		    else if(strcmp(credAdd, "y") == 0) credPrint("z");
		    else credPrint("w");
		}
		else if(keyMode == 1)
		{
		    if(strcmp(credAdd, "W") == 0) credPrint("X");
		    else if(strcmp(credAdd, "X") == 0) credPrint("Y");
		    else if(strcmp(credAdd, "Y") == 0) credPrint("Z");
		    else credPrint("W");
		}
		else if(keyMode == 2)
		{
		    credPrint("9");
		}
	    }
	    else if(*dataReceived == NUM81 || *dataReceived == NUM82) //row 4 column 1
	    {
		//printf("Resetting LCD usart connection...\n");
		LCD_reset();
		//printf("LCD usart connection reset\n");
		if(keyMode == 0) keyMode = 1;
		else if(keyMode == 1) keyMode = 0;
		else if(keyMode == 2) keyMode = 0;
		keyboardDraw(keyMode);
		id = 0; credNext();
	    }
	    else if(*dataReceived == NUM83 || *dataReceived == NUM84) //row 4 column 2
	    {
		if (keyMode == 0 || keyMode == 1)
		    credNext();
		else if (keyMode == 2)
		{
		    id = 10;
		    
		    if (oldID > 0 && id != oldID)
			credNext();
		    
		    credPrint("0");
		}
	    }
	    else if(*dataReceived == NUM85 || *dataReceived == NUM86) //row 4 column 3
	    {
		credRemove();
	    }
	    else if(*dataReceived <= NUM7) //submit button
	    {
		if(cursorFull == 1) sprintf(credString, "%s%s", credString, credAdd);
				    
		if(credMode == 0)
		{
		    memset(usernameString, 0, 20*sizeof(char));
		    sprintf(usernameString, "%s", credString);
		}
		else if(credMode == 1)
		{
		    memset(passwordString, 0, 20*sizeof(char));
		    strcpy(passwordString, credString);
		}
#ifdef DEBUG_MODE		
		printf("The Google user name string is:\n");
		printf("%s\n", usernameString);
#endif		
		showOptions(); //go to the options screen if the options button is pressed
		return;
	    }
	}
    }
}

//display the time zone entry screen and get time zone input from the user
void timezoneEntry()
{
    int tzKeyMode = *tzShift < 0 ? 3 : 4;
    int tzSign = *tzShift < 0 ? -1 : 1;
    //int i = 0;
    //char buf[20];
    
    //this variable stores data received from the LCD
    u_int* dataReceived;

    //this flag is used to indicate whether the cursor has a char above it
    cursorFull = 0;

    //clear the credString
    memset(credString, 0, 20*sizeof(char));

    //reset input text position
    credPosition = 0;

    clearScreen();

    //draw title text
    selectFont(2);
    setColor(0xC0); //blue
    setLocation(5, 7);
    printString("Please enter your UTC shift:");

    //draw enter button
    setLocation(180, 0);
    drawBox(60, 27);
    setLocation(191, 6);
    printString("Submit");

    //draw text field
    setLocation(30, 30);
    drawBox(180, 21);

    //draw cursor
    setLocation(38, 46);
    drawLine(46, 46);

    keyboardDraw(tzKeyMode);
/*    sprintf(buf, "%d", abs(*tzShift));
 - 
 -     for (i = 0; i < strlen(buf); ++i)
 -     {
 - 	char tmp[2];
 - 	tmp[0] = buf[i];
 - 	tmp[1] = 0;
 - 	
 - 	credPrint(tmp);
 - 	credNext();
 -     }*/

    while(1)
    {
	if(LCD_read(dataReceived))
	{
	    //when a button is pressed, process the command accordingly
	    if(*dataReceived == NUM33 || *dataReceived == NUM34) //row 1 column 1
	    {
		credPrint("1");

		credNext();
	    }
	    else if(*dataReceived == NUM35 || *dataReceived == NUM36) //row 1 column 2
	    {
		credPrint("2");

		credNext();
	    }
	    else if(*dataReceived == NUM37 || *dataReceived == NUM38) //row 1 column 3
	    {
		credPrint("3");

		credNext();
	    }
	    else if(*dataReceived == NUM49 || *dataReceived == NUM50) //row 2 column 1
	    {
		credPrint("4");

		credNext();
	    }
	    else if(*dataReceived == NUM51 || *dataReceived == NUM52) //row 2 column 2
	    {
		credPrint("5");

		credNext();
	    }
	    else if(*dataReceived == NUM53 || *dataReceived == NUM54) //row 2 column 3
	    {
		credPrint("6");

		credNext();
	    }
	    else if(*dataReceived == NUM65 || *dataReceived == NUM66) //row 3 column 1
	    {
		credPrint("7");

		credNext();
	    }
	    else if(*dataReceived == NUM67 || *dataReceived == NUM68) //row 3 column 2
	    {
		credPrint("8");

		credNext();
	    }
	    else if(*dataReceived == NUM69 || *dataReceived == NUM70) //row 3 column 3
	    {
		credPrint("9");

		credNext();
	    }
	    else if(*dataReceived == NUM81 || *dataReceived == NUM82) //row 4 column 1
	    {
		tzSign *= -1;
		//printf("Resetting LCD usart connection...\n");
		LCD_reset();
		//printf("LCD usart connection reset\n");
		if(tzKeyMode == 3) tzKeyMode = 4;
		else if(tzKeyMode == 4) tzKeyMode = 3;
		keyboardDraw(tzKeyMode);
	    }
	    else if(*dataReceived == NUM83 || *dataReceived == NUM84) //row 4 column 2
	    {
		credPrint("0");

		credNext();
	    }
	    else if(*dataReceived == NUM85 || *dataReceived == NUM86) //row 4 column 3
	    {
		credRemove();
	    }
	    else if(*dataReceived <= NUM7) //submit button
	    {
		if(cursorFull == 1) sprintf(credString, "%s%s", credString, credAdd);

		updateTZ(tzSign * atoi(credString));
		
		showOptions(); //go to the options screen if the options button is pressed
		
		return;
	    }
	}
    }
}

//the main function containing the main loop
int main()
{
    //this is a string used for debugging
    //char debug_string[40];
    
    //this variable stores data received from the LCD
    u_int* dataReceived;

    //initialize flags, turn off alarm and set mode to main screen
    alarmSounding = 0;
    currentMode = 0;
    alarmShouldBeOn = 0;

    //make space for the various string
    credString = (char *)calloc(20, sizeof(char));
    //usernameString = (char *)calloc(40, sizeof(char));
    //passwordString = (char *)calloc(20, sizeof(char));

    //init the weight sensors
    setupWeightSensors();

    //init the LCD over the usart
    LCD_init();
    lightOn();

    //clear the screen
    setColor(0xFF);
    clearScreen();

    //initialize the syncTime struct
    syncTime.firstRun = 1;
    alarmBufferPointer = 0;
    
    //initialize the clock
    dateTimeInit();

    // read current date and time from Internet source and store in RTC
    sync();

    //read current date and time
    dateTimeRead();

    // display the main screen
    mainScreen();
    
    //the main loop
    while(1)
    {
	if(currentMode == 0) //if part implements the main screen functionality
	{
	    //read button press data from the LCD
	    if(LCD_read(dataReceived))
	    {
		//////////////////////////////DEBUG/////////////////////////////////
		//sprintf(debug_string, "dataReceived is: %d\n", *dataReceived);
		//dprint(debug_string);
		////////////////////////////////////////////////////////////////////
	
		if(*dataReceived <= NUM38)
		{
		    //printf("Resetting LCD usart connection...\n");
		    LCD_reset();
		    //printf("LCD usart connection reset\n");
		}
		//toggle the LCD light when the light button is pressed
		if(*dataReceived == NUM80 || *dataReceived == NUM81)
		{
		    //toggle light and button text
		    selectFont(2);
 		    setColor(0xFF);
 		    setLocation(1, 131);
		    fillBox(59, 28);
		    setColor(0xC0); //blue
		    setLocation(8, 138);
		    if(lightStatus == 0)
		    {
			lightOn();
			lightStatus = 1;
			
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
		/* else if(*dataReceived == NUM82 || *dataReceived == NUM83)
		{
		    currentMode = 1; //update to weather mode
		    showWeather(); //go to the weather screen
		    continue;
		    } */ /* forget about weather for now */
		else if(*dataReceived == NUM82 || *dataReceived == NUM83 || *dataReceived == NUM84)
		{
		    currentMode = 2; //update to options mode
		    showOptions(); //go to the options screen if the options button is pressed
		    continue;
		}
		else if(*dataReceived == NUM85 || *dataReceived == NUM86)
		{
		    sync();
		    mainScreen();
		}
	    }
    
	    //read current date and time
	    dateTimeRead();
	    
	    //update time display
	    printTime();
    
	    //update date display
	    printDate();

	    /* sync every hour */
	    if (myRTC->time_int.min == 0)
		sync();
    
	    // do alarm work
	    doAlarm();
	} //end of currentMode == 0 (main screen)

	if(currentMode == 1) //implements the functionality of the weather screen
	{
	    //read button press data from the LCD
	    if(LCD_read(dataReceived))
	    {
		//if the back to main screen button is pressed, go back to the main screen
		if(*dataReceived >= NUM80 && *dataReceived <= NUM86)
		{
		    currentMode = 0;
		    // display the main screen
		    mainScreen();
		    continue;
		}
	    }    
	} //end of currentMode == 1 (weather)

	if(currentMode == 2) //implements the functionality of the options screen
	{   
	    //read button press data from the LCD
	    if(LCD_read(dataReceived))
	    {
		//if username button is pressed, prompt user to enter username
		if(*dataReceived >= NUM17 && *dataReceived <= NUM21)
		{
		    credEntry(0);
		}
		//if password button is pressed, prompt user to enter password
		else if(*dataReceived >= NUM33 && *dataReceived <= NUM37)
		{
		    credEntry(1);
		}
		//if time zone button is pressed, prompt user to enter time zone
		else if(*dataReceived >= NUM49 && *dataReceived <= NUM53)
		{
		    timezoneEntry();
		}
		//if back button is pressed, go back to the main screen
		else if(*dataReceived >= NUM65 && *dataReceived <= NUM69)
		{
		    currentMode = 0;
		    // display the main screen
		    mainScreen();
		    continue;
		}
	    }
	    
	} //end of currentMode == 2 (options)
	
    } //while(1)
    
}
