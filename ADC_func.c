#include "ADC_func.h"

void ADC_init(){
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	ADC1 -> SMPR |= ADC_SMPR_SMPR_1 | ADC_SMPR_SMPR_2 ;
	ADC1 -> CR |= ADC_CR_ADEN;
	while(!(ADC1->ISR&ADC_ISR_ADRDY));
	ADC1 -> IER |= ADC_IER_EOCIE;
//	ADC1->CR2 |= ADC_CR2_CAL;
//	while (!(ADC1->CR2 & ADC_CR2_CONT));
//	ADC1 -> CR1 = ADC_CR1_EOCIE;
//	ADC1 -> CR2 |= ADC_CR2_EXTSEL | ADC_CR2_EXTEN;


}

unsigned int ADC_read(uint32_t chanel){
	 ADC1->CHSELR = chanel;
	ADC1 -> CR |= ADC_CR_ADSTART;
	while (!(ADC1 -> ISR & ADC_ISR_EOC)){

	}
	return ADC1 -> DR;
}

