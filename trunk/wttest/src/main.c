#ifdef SEMIHOSTING
#include <stdio.h>
#endif

#include "parts/m55800/lib_m55800.h"
#include "periph/adc/lib_adc.h"

int main()
{
    u_int channel[4];
    
    at91_adc_open(&ADC0_DESC, ADC_TRGEN_DIS | ADC_10_BIT_RES | ADC_NORMAL_MODE | ADC_PRESCAL);
    at91_adc_channel_open(&ADC0_DESC, ADC_CH2);

    at91_adc_trig_cmd(&ADC0_DESC, ADC_START);

    while ((at91_adc_get_status(&ADC0_DESC) & ADC_EOC2) == 0) {}
    at91_adc_read(&ADC0_DESC, channel);

    printf("%d\n", channel[2]);    

    at91_adc_channel_close(&ADC0_DESC, ADC_CH2);
    at91_adc_close(&ADC0_DESC);
    
    return 0;
}
