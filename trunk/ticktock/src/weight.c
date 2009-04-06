#ifdef SEMIHOSTING
#include <stdio.h>
#endif

#include "parts/m55800/lib_m55800.h"
#include "periph/adc/lib_adc.h"

#include "main.h"

int alarmActive(double lowerThresh, double upperThresh);
void beepOff();
void beepOn();

extern int alarmSounding;

void setupWeightSensors()
{
    at91_pio_open(&PIOB_DESC, PIOB0_MASK, PIO_OUTPUT);
    beepOff();
}

int alarmActive(double lowerThresh, double upperThresh)
{
    int i = 0;
    double sensor = 0.0;
    int retVal = 0;
    
    at91_adc_open(&ADC1_DESC, ADC_TRGEN_DIS | ADC_10_BIT_RES | ADC_NORMAL_MODE | ADC_PRESCAL);
    at91_adc_channel_open(&ADC1_DESC, ADC_CH0);
    at91_adc_channel_open(&ADC1_DESC, ADC_CH1);
    at91_adc_channel_open(&ADC1_DESC, ADC_CH2);
    at91_adc_channel_open(&ADC1_DESC, ADC_CH3);

    for (i = 0; i < WEIGHT_SAMPLES; ++i)
    {
	u_int channel[4];
	int j = 0;
	at91_adc_trig_cmd(&ADC1_DESC, ADC_START);

	// wait for a sample from each sensor
	while (((at91_adc_get_status(&ADC1_DESC) & ADC_EOC0) == 0) || ((at91_adc_get_status(&ADC1_DESC) & ADC_EOC1) == 0) || ((at91_adc_get_status(&ADC1_DESC) & ADC_EOC2) == 0) || ((at91_adc_get_status(&ADC1_DESC) & ADC_EOC3) == 0)) {}
	at91_adc_read(&ADC1_DESC, channel);

	for (j = 0; j < 4; ++j)
	    sensor += channel[j];
    }

    at91_adc_channel_close(&ADC1_DESC, ADC_CH3);
    at91_adc_channel_close(&ADC1_DESC, ADC_CH2);
    at91_adc_channel_close(&ADC1_DESC, ADC_CH1);
    at91_adc_channel_close(&ADC1_DESC, ADC_CH0);
    at91_adc_close(&ADC1_DESC);

    sensor /= WEIGHT_SAMPLES;

    if (sensor < lowerThresh || sensor > upperThresh)
	retVal = 1;
    
    return retVal;
}

void beepOff()
{
    at91_pio_write(&PIOB_DESC, PIOB0_MASK, PIO_CLEAR_OUT);
    alarmSounding = 0;
}

void beepOn()
{
    at91_pio_write(&PIOB_DESC, PIOB0_MASK, PIO_SET_OUT);
    alarmSounding = 1;
}
