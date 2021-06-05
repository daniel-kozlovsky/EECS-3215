/*
 * Part1.c
 *
 *  Created on: Mar 2, 2020
 *      Author: dankoz
 */


#include "LPC802.h"
#include "clock_config.h"


#define LED1 (9)
int main0(void)
{
	const unsigned long COUNT = 1UL<<19;

	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK |
									SYSCON_SYSAHBCLKCTRL0_GPIO_INT_MASK);

	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK |
								SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);

	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK|
								SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);

	GPIO->DIRSET[0] = (1UL<<LED1);

	while(1)
	{
		for(unsigned long l = 0; l < COUNT; l++)
		{

		}
		GPIO->NOT[0] = (1UL<<LED1);
	}
	return 0;
}
