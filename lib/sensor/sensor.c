#include <avr/io.h>
#include <util/delay.h>

void initADC()
{
    ADMUX = ( 1 << REFS0 ) | ( 1 << MUX2 );

    ADCSRA = ( 1 << ADEN ) | ( 1 << ADPS2 ) | ( 1 << ADPS1 ) | ( 1 << ADPS0 ); 
}

uint16_t readADC(uint8_t channel)
{
    if (channel > 7)
        return;

    ADMUX = ( ADMUX & 0xF0 ) | channel;

    ADCSRA |= ( 1 << ADSC );

    while ( ADCSRA & ( 1 << ADSC ));

    uint16_t adc_value = ADCL;
    adc_value |= ( ADCH << 0 );

    return adc_value;
}