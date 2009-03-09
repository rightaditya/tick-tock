#ifdef SEMIHOSTING
#include <stdio.h>
#endif

#include "periph/usart/lib_usart.h"
#include "parts/m55800/lib_m55800.h"

void Usart_init (void);
//AT91PS_USART COM0;

#define LCD_BAUD	115200
#define TIMEOUT		50

static const char clr_str[] = "$8";
static const char PDC_header[]=
{
"\n\r  *** I'm Testing This Frakking LCD lol~~ ***\n\r"
};

//*----------------------------------------------------------------------------
//* Function Name       : Usart_init
//* Object              : USART initialization 
//* Input Parameters    : none
//* Output Parameters   : TRUE
//*----------------------------------------------------------------------------
void Usart_init ( void )
//* Begin
{
    at91_usart_open(&USART1_DESC, US_ASYNC_MODE, LCD_BAUD, 0);
    
    /*
	   COM0= AT91C_BASE_US1;

    // Define RXD and TXD as peripheral
       AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA,AT91C_PA19_RXD1 | AT91C_PA18_TXD1,0);

    // First, enable the clock 
    	AT91F_APMC_EnablePeriphClock ( AT91C_BASE_APMC, 1<<AT91C_ID_US1 ) ;
	
    // Reset receiver and transmitter
       COM0->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX;

    // Reset Status Bits (PARE, FRAME, OVRE and RXBRK)
       COM0->US_CR = AT91C_US_RSTSTA;

    // Usart Configure
       AT91F_US_Configure (COM0, MCK,AT91C_US_ASYNC_MODE, US_SPEED, 0);
	
    // Enable usart 
        COM0->US_CR = AT91C_US_RXEN | AT91C_US_TXEN;
    	
    // Enable USART IT error and AT91C_US_ENDRX
       AT91F_US_EnableIt(COM0,AT91C_US_TIMEOUT | AT91C_US_FRAME | AT91C_US_OVRE |AT91C_US_RXRDY);

    // open Usart 1 interrupt
       //AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, AT91C_ID_US1, USART_INTERRUPT_LEVEL,AT91C_AIC_SRCTYPE_INT_LEVEL_SENSITIVE, usart_asm_irq_handler); 
       //AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_US1); 

    // Wait for USART CLOCK ready
       while ( !( COM0->US_CSR & AT91C_US_TXRDY ));
	
   // Arm time out after 255 * 4  bits time 
	  COM0->US_RTOR = TIMEOUT;
   // Enable usart SSTO
	  COM0->US_CR = AT91C_US_STTTO;
		
   // first
 	  //AT91F_US_SendFrame(COM0,(char *)atmel_header,sizeof(atmel_header));
   	  //AT91F_US_EnableIt(COM0, AT91C_US_ENDTX );*/
	
//* End
}

int main()
{
    unsigned int   loop_count ;

    //* Init
    loop_count = 0 ;

    //* Init Usart
    Usart_init();

    for (;;)
    {       
        loop_count ++ ;

        if (loop_count == 100)
        {
             loop_count=0;
	     while((at91_usart_get_status(&USART1_DESC) & US_TXRDY) ==0) {}
	     at91_usart_write (&USART1_DESC, '!') ; 
	     /*AT91F_US_SendFrame(COM0,(char *)clr_str,sizeof(clr_str));
	     //AT91F_US_SendFrame(COM0,(char *)PDC_header,sizeof(PDC_header));
	     AT91F_US_EnableIt(COM0, AT91C_US_ENDTX );*/
        }
    }

}

