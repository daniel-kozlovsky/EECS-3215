#include "LPC802.h"

#define LED (9)
#define BUTTON_USER1 (8)


void PIN_INT0_IRQHandler(void)
{
	if(PINT->IST & (1<<0))
	{
		PINT->IST = (1<<0);

		GPIO->NOT[0] = (1UL<<LED);
	}
	else
	{
		asm("NOP");
	}
}

int main(void) {

	__disable_irq();
	NVIC_DisableIRQ(PIN_INT0_IRQn);
	// Turn on the GPIO system.
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK |
								SYSCON_SYSAHBCLKCTRL0_GPIO_INT_MASK);

	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK |
							SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);

	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK|
							SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);

	GPIO->DIRCLR[0] = (1UL<<BUTTON_USER1);
	GPIO->CLR[0] = (1UL<<LED);
	GPIO->DIRSET[0] = (1UL<<LED);

	SYSCON->PINTSEL[0] = BUTTON_USER1;

	PINT->ISEL = 0x00;

	//Falling edge interrupt
	PINT->CIENR = 0b00000001;
	PINT->SIENF = 0b00000001;

	PINT->IST = 0xFF;

	NVIC_EnableIRQ(PIN_INT0_IRQn);
	__enable_irq();

 // loop infinitely
	 while(1)
	 {
		 asm("NOP");
	 }
	 return 0;



} // end of main
