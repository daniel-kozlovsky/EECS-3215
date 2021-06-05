/*
 * Part1.c
 *
 *  Created on: Mar 2, 2020
 *      Author: dankoz
 */


#include "LPC802.h"
#include "clock_config.h"


#define LED1 (9)

void SysTick_Handler(void)
{
	GPIO->NOT[0] = (1UL<<LED1);
}
int main1(void)
{
	__disable_irq();
	NVIC_DisableIRQ(SysTick_IRQn);

	SYSCON->MAINCLKSEL = (0x0<<SYSCON_MAINCLKSEL_SEL_SHIFT);

	SYSCON->MAINCLKUEN &= ~(0x1);
	SYSCON->MAINCLKUEN |= 0X1;

	BOARD_BootClockFRO24M();//12mhz

	SysTick_Config(2400000);// 0.2 sec @12mhz

	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK |
									SYSCON_SYSAHBCLKCTRL0_GPIO_INT_MASK);

	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK);

	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK);

	GPIO->CLR[0] = (1UL<<LED1);
	GPIO->DIRSET[0] = (1UL<<LED1);

	NVIC_EnableIRQ(SysTick_IRQn);
	__enable_irq();

	while(1)
	{
		__asm("nop");
	}

	return 0;
}
