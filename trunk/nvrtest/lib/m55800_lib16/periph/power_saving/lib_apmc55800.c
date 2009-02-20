//*--------------------------------------------------------------------------------------
//*         ATMEL Microcontroller Software Support  -  ROUSSET  -
//*--------------------------------------------------------------------------------------
//* The software is delivered "AS IS" without warranty or condition of any
//* kind, either express, implied or statutory. This includes without
//* limitation any warranty or condition with respect to merchantability or
//* fitness for any particular purpose, or against the infringements of
//* intellectual property rights of others.
//*--------------------------------------------------------------------------------------
//* File Name           : lib_apmc55800.c
//* Object              : Advanced Power Management Controller Library.
//*
//* 1.0 22/05/00  PF    : Creation
//*--------------------------------------------------------------------------------------
#include    "periph/power_saving/lib_power_save.h"

//*--------------------------------------------------------------------------------------
//* Function Name       : at91_clock_set_mode
//* Object              : Set System Clock Mode.
//* Input Parameters    : <mode> =
//* Output Parameters   : none
//* Functions called    : at91_error
//*--------------------------------------------------------------------------------------
void at91_clock_set_mode ( u_int mode )
//* Begin
{
    //* Depending on the required mode
    switch (mode)
    {
        //* Idle mode required
        case PS_MODE_IDLE:
            //* Write the System Clock Disable Register
            APMC_BASE->APMC_SCDR = APMC_ARM7DIS ;
            break ;

        //* Active all peripheral clocks
        case PS_ALL_PERIPH_ACTIVE:
            //* Enable all the peripheral clocks
            APMC_BASE->APMC_PCER = 0xFFFFFFFF ;
            break ;

        //* Desactive all peripheral clocks
        case PS_ALL_PERIPH_INACTIVE:
            //* Disable all the peripheral clocks
            APMC_BASE->APMC_PCDR = 0xFFFFFFFF ;
            break ;

        //* Unknown
        default:
            //* Run AT91 Library error function
            at91_error (__FILE__, __LINE__) ;
    }
    //* EndSwitch
}
//* End

//*--------------------------------------------------------------------------------------
//* Function Name       : at91_clock_open
//* Object              : Enable the peripheral clock
//* Input Parameters    : <periph_id> = peripheral identifier
//* Output Parameters   : none
//* Functions called    : none
//*--------------------------------------------------------------------------------------
void at91_clock_open ( u_int periph_id )
//* Begin
{
    //* Write the Peripheral Clock Enable Register
    APMC_BASE->APMC_PCER = (1 << periph_id) ;
}
//* End

//*--------------------------------------------------------------------------------------
//* Function Name       : at91_clock_close
//* Object              : Disable the clock of a Peripheral
//* Input Parameters    : <periph_id> = peripheral identifier
//* Output Parameters   : none
//* Functions called    : none
//*--------------------------------------------------------------------------------------
void at91_clock_close ( u_int periph_id )
//* Begin
{
    //* Write the Peripheral Clock Disable Register
    APMC_BASE->APMC_PCDR = (1 << periph_id) ;
}
//* End

//*--------------------------------------------------------------------------------------
//* Function Name       : at91_clock_get_status
//* Object              : Return the Peripheral clock status
//* Input Parameters    : <periph_id> = peripheral identifier
//* Output Parameters   : none
//* Functions called    : none
//*--------------------------------------------------------------------------------------
u_int at91_clock_get_status ( u_int periph_id )
//* Begin
{
    //* Return the Peripheral Clock Status Register
    return ( APMC_BASE->APMC_PCSR & (1<<periph_id) ) ;
}
//* End


//*--------------------------------------------------------------------------------------
//* Function Name       : at91_clock_get_pll_status
//* Object              : Return the APMC Status Register
//* Input Parameters    : none
//* Output Parameters   : none
//* Functions called    : none
//*--------------------------------------------------------------------------------------
u_int at91_clock_get_pll_status ( void )
//* Begin
{
    //* Return the PMC Status Register
    return ( APMC_BASE->APMC_SR ) ;
}
//* End

//*--------------------------------------------------------------------------------------
//* Function Name       : at91_clock_generator_mode
//* Object              : Set Master Clock selection, Main OSC & PLL
//* Input Parameters    : none
//* Output Parameters   : none
//* Functions called    : none
//*--------------------------------------------------------------------------------------
void at91_clock_generator_mode (u_int mode )
//* Begin
{
    APMC_BASE->APMC_CGMR = mode ;
}
//* End
//*--------------------------------------------------------------------------------------
//* Function Name       : at91_clock_generator_state NEW
//* Object              : Return Master Clock selection, Main OSC & PLL
//* Input Parameters    : none
//* Output Parameters   : none
//* Functions called    : none
//*--------------------------------------------------------------------------------------
u_int at91_clock_generator_state (void)
//* Begin
{
    //* Return the CGMR state
    return ( APMC_BASE->APMC_CGMR ) ;
}
//* End

//*--------------------------------------------------------------------------------------
//* Function Name       : at91_power_mode
//* Object              : Set Power Mode register
//* Input Parameters    : none
//* Output Parameters   : none
//* Functions called    : none
//*--------------------------------------------------------------------------------------
void at91_power_mode (u_int mode )
//* Begin
{
    APMC_BASE->APMC_PMR = mode ;
}
//* End

//*--------------------------------------------------------------------------------------
//* Function Name       : at91_trig_cmd
//* Object              : Trig Power Control register
//* Input Parameters    : none
//* Output Parameters   : none
//* Functions called    : none
//*--------------------------------------------------------------------------------------
void at91_trig_cmd (u_int cmd )
//* Begin
{
    APMC_BASE->APMC_PCR = cmd ;
}
//* End

//*--------------------------------------------------------------------------------------
//* Function Name       : at91_interrupt_enable NEW
//* Object              : Enable Main Oscillator or PLL source interrupt
//* Input Parameters    : none
//* Output Parameters   : none
//* Functions called    : none
//*--------------------------------------------------------------------------------------
void at91_interrupt_enable (u_int interrupt_source)
{
        APMC_BASE->APMC_IER = interrupt_source ;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : at91_interrupt_disable NEW
//* Object              : Disable Main Oscillator or PLL source interrupt
//* Input Parameters    : none
//* Output Parameters   : none
//* Functions called    : none
//*--------------------------------------------------------------------------------------
void at91_interrupt_disable (u_int interrupt_source)
{
        APMC_BASE->APMC_IDR = interrupt_source ;
}
