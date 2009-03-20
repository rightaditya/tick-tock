#ifdef SEMIHOSTING
#include <stdio.h>
#endif

#include "parts/m55800/lib_m55800.h"
#include "periph/adc/lib_adc.h"

#define WEIGHT_CALIBRATION_SAMPLES 50
#define WEIGHT_SAMPLES 10
#define WEIGHT_ZONE_SPLIT 0.8

int activateAlarm(double lowerThresh, double upperThresh);

int main()
{
    while (1)
    {
	int i = activateAlarm(100, 500);

	printf("%d\n", i);
    }
    
    return 0;
}

int activateAlarm(double lowerThresh, double upperThresh)
{
    int i = 0;
    double sensor[4];
    int retVal = 0;

    for (i = 0; i < 4; ++i)
	sensor[i] = 0.0;
    
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
	while (((at91_adc_get_status(&ADC1_DESC) & ADC_EOC0) == 0) || ((at91_adc_get_status(&ADC1_DESC) & ADC_EOC1) == 0) || ((at91_adc_get_status(&ADC1_DESC) & ADC_EOC2) == 0) || ((at91_adc_get_status(&ADC1_DESC) & ADC_EOC3) || 0)) {}
	at91_adc_read(&ADC1_DESC, channel);

	for (j = 0; j < 4; ++j)
	    sensor[j] += channel[j];
    }

    at91_adc_channel_close(&ADC1_DESC, ADC_CH3);
    at91_adc_channel_close(&ADC1_DESC, ADC_CH2);
    at91_adc_channel_close(&ADC1_DESC, ADC_CH1);
    at91_adc_channel_close(&ADC1_DESC, ADC_CH0);
    at91_adc_close(&ADC1_DESC);

    for (i = 0; i < 4; ++i)
	sensor[i] /= WEIGHT_SAMPLES;

    if (sensor[0] < lowerThresh || sensor[1] < lowerThresh || sensor[2] < lowerThresh || sensor[3] < lowerThresh)
	retVal = 1;
    else if (sensor[0] > upperThresh || sensor[1] > upperThresh || sensor[2] > upperThresh || sensor[3] > upperThresh)
	retVal = 1;
    else
	retVal = 0;

    return retVal;
}
