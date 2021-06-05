#include "LPC802.h"

#define LED (9)
#define BUTTON_USER1 (8)


int main(void) {
	// Turn on the GPIO system.
	SYSCON->SYSAHBCLKCTRL0 |= SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK;
	SYSCON->PRESETCTRL0 &= ~SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK;
	SYSCON->PRESETCTRL0 |= SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK;

	GPIO->DIRCLR[0] = (1UL<<BUTTON_USER1);
	GPIO->CLR[0] = (1UL<<LED);
	GPIO->DIRSET[0] = (1UL<<LED);
 // loop infinitely
	 while(1)
	 {
		 //asm("NOP");
		 if(GPIO->B[0][BUTTON_USER1] & 1)
		 {
			GPIO->CLR[0] = (1UL<<LED);//turn on
		 }
		 else
		 {
			 GPIO->SET[0] = (1UL<<LED);
		 }

	 }
	 return 0;



} // end of main
