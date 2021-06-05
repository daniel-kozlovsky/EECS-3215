/*

#include "clock_config.h"
#include "LPC802.h"

#define CTIMER_FREQ (12000000)
#define PWM_PERCENT (0.75)// value b/w 0 and 1.
#define CTIMER_MATCH (CTIMER_FREQ*(1-PWM_PERCENT))

int main(void)
{
	 // assume 12 MHz FRO clock
	 // enable switch matrix
	 SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_SWM_MASK);
	 // Set switch matrix
	 // ---> PINASSIGN4 T0-MAT0 is (bits 7:0) is 0x9
	 // Connect CTimer0 channel 0 to an external LED pin
	 SWM0->PINASSIGN.PINASSIGN4 &= ~(0xff); // clear the bottom 8 bits
	 SWM0->PINASSIGN.PINASSIGN4 |= (0x9);// put 0x9 is the bottom 8 bits.
	 // this should make the GPIO with the Green LED pair the output pin. (PB 9)
	// Enable CTIMER clock
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_CTIMER0_MASK);
	// Reset the CTIMER module
	// 1. Assert CTIMER-RST-N
	// 2. Clear CTIMER-RST-N
	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_CTIMER0_RST_N_MASK); // Reset
	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_CTIMER0_RST_N_MASK); // clear the reset.
	// Enable PWM mode on the channel 0
	// **************************************
	// 1. EMR bit 0 set to 1 (External Match Register)
	// 2. PWMEN0 (PWMC bit 0) set to "PWM"
	// ****************************************
	CTIMER0->EMR |= CTIMER_EMR_EM0_MASK; // (rule complicated)
	CTIMER0->PWMC |= CTIMER_PWMC_PWMEN0_MASK; // 0 is match mode; 1 is PWM mode.
	// Clear all the Channel 0 bits in the MCR
	CTIMER0->MCR &= ~(CTIMER_MCR_MR0R_MASK | CTIMER_MCR_MR0S_MASK | CTIMER_MCR_MR0I_MASK);
	// Reset the counter when match on channel 3
	// ***********************************
	// MCR bit MR3R gets set to 1
	// ***********************************
	CTIMER0->MCR |= CTIMER_MCR_MR3R_MASK; // MR3R bit to 1
	// Match on channel 3 will define the PWM period
	// If we use the 12MHz internal clock, then 12000000 will make it 1 Hz.
	// ************************************
	CTIMER0->MR[3] = CTIMER_FREQ; //
	// This will define the PWM pulse off time.
	// If this is 75% of MR[3] value you'll get 25 Duty Cycle.
	// Make this freq * (1-PWM_DUTY_FACTOR).
	CTIMER0->MR[0] = CTIMER_MATCH; // DUTY FACTOR is 0.75
	 // Start the timer:
	 // **************************************************
	 // 1. CTIMER0->TCR CEN bit (bit 0) got set to ENABLED
	 // 2. The timer counter (TC) will contain values... CTIMER0->TC
	 // **************************************************
	 CTIMER0->TCR |= CTIMER_TCR_CEN_MASK; // 0 is disabled. 1 is enabled.
	 // at this point, the TCR ENable bit gets set to 1
	 // and then, automatically, we should see the timer TCVAL value change.
	 // that should confirm that the timer is actually running.
	  Enter an infinite loop, just incrementing a counter.
	 while(1) {
		 // nothing
		 asm("NOP");
	 }
	 return 0 ;
}
*/
