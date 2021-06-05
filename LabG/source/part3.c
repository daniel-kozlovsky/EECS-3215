/*
 * part2.c
 *
 *  Created on: Mar 2, 2020
 *      Author: dankoz
 */

#include "LPC802.h"
#include "clock_config.h"

#define LED_USER1 (8)
#define LED_USER2 (9)
#define MRT_REPEAT (0)
#define MRT_ONESHOT (1)
#define MRT_ONESHOT_BusStall (2)
#define MRT_GFLAG0 (0)
#define MRT_GFLAG1 (1)

#define MRT_CHAN0 (0)
#define MRT_CHAN1 (1)

#define SEGMENT_PERIOD_TICKS (300000) //TWO IRQS IN COUNTER TOTAL //100hz blink

#define INIT_COUNT_IRQ_CH0 (SEGMENT_PERIOD_TICKS/2)
#define INIT_PWM_DUTY_5 0.05
#define INIT_PWN_DUTY_95 0.95
void MRT0_IRQHandler(void)
{
	int static chan = 0;

	float static duty_factor = INIT_PWM_DUTY_5;

	int static pwm_state = 1; //led on

	if(MRT0->IRQ_FLAG & (1<<MRT_GFLAG0))
	{
		chan = 0;
	}
	else
	{
		chan = 1;
	}

	MRT0->CHANNEL[chan].STAT = MRT_CHANNEL_STAT_INTFLAG_MASK;


	if(pwm_state == 1)
	{

		GPIO->CLR[0] = (1UL<<LED_USER1);//on
		GPIO->SET[0] = (1UL<<LED_USER2); //TURN off LED
		MRT0->CHANNEL[MRT_CHAN0].INTVAL = (uint32_t)(INIT_COUNT_IRQ_CH0 * (1-duty_factor))
				& ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

		pwm_state = 0;
	}
	else
	{
		GPIO->SET[0] = (1UL<<LED_USER1); //TURN off LED
		GPIO->CLR[0] = (1UL<<LED_USER2);
		MRT0->CHANNEL[MRT_CHAN0].INTVAL = (uint32_t)(INIT_COUNT_IRQ_CH0 * (duty_factor))
						& ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

		pwm_state = 1;
	}
	return;
}

int main(void)
{
	__disable_irq();
	NVIC_DisableIRQ(MRT0_IRQn);

	//Main clock to 12Mhz

	SYSCON->MAINCLKSEL = (0x0<<SYSCON_MAINCLKSEL_SEL_SHIFT);

	SYSCON->MAINCLKUEN &= ~(0x1);
	SYSCON->MAINCLKUEN |= 0x1;

	BOARD_BootClockFRO30M();//15Mhz FRO freq

	//GPIO
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK);//ON
	GPIO->SET[0] = (1UL<<LED_USER1);//OFF
	GPIO->DIRSET[0] = (1UL<<LED_USER1);//OUTPUT


	GPIO->SET[0] = (1UL<<LED_USER2);//OFF
	GPIO->DIRSET[0] = (1UL<<LED_USER2);//OUTPUT

	//MRT setup
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_MRT_MASK);//MRT ON

	//RESET
	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_MRT_RST_N_MASK);//RESET
	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_MRT_RST_N_MASK);//CLEAR BIT

	//chan0 into oneshot
	MRT0->CHANNEL[MRT_CHAN0].CTRL = (MRT_REPEAT << MRT_CHANNEL_CTRL_MODE_SHIFT
			| MRT_CHANNEL_CTRL_INTEN_MASK);

	//MRT0->CHANNEL[MRT_CHAN1].CTRL = (MRT_ONESHOT << MRT_CHANNEL_CTRL_MODE_SHIFT
	//			| MRT_CHANNEL_CTRL_INTEN_MASK);

	//START
	MRT0->CHANNEL[MRT_CHAN0].INTVAL = INIT_COUNT_IRQ_CH0 & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);
	//MRT0->CHANNEL[MRT_CHAN1].INTVAL = IVALUE_CH1 & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

	//ENABLE INTERRUPTS
	NVIC_EnableIRQ(MRT0_IRQn);
	__enable_irq();


	while(1)
		{
			__asm("NOP");
		}
		return 0;
}
