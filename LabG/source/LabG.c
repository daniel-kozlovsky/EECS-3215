/*
 * LabG.c
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
#define IVALUE_CH0 (60000000) // (counts) irq desired period
#define IVALUE_CH1 (120000000)


void MRT0_IRQHandler0(void)
{
	int chan = 0;
	if(MRT0->IRQ_FLAG & (1<<MRT_GFLAG0))
	{
		chan = 0;
	}
	else
	{
		chan = 1;
	}

	MRT0->IRQ_FLAG = MRT_CHANNEL_STAT_INTFLAG_MASK;

	if(chan == 0)
	{
		GPIO->NOT[0] = (1UL<<LED_USER1);//toggle
	}
	else
	{
		GPIO->NOT[0] = (1UL<<LED_USER2);//toggle
	}
}


int main0(void)
{
	//disable interrupts
	__disable_irq();//global
	NVIC_DisableIRQ(MRT0_IRQn); //MRT0 interrupt off

	//Main clock to 12Mhz

	SYSCON->MAINCLKSEL = (0x0<<SYSCON_MAINCLKSEL_SEL_SHIFT);

	SYSCON->MAINCLKUEN &= ~(0x1);
	SYSCON->MAINCLKUEN |= 0x1;

	BOARD_BootClockFRO24M();//12Mhz FRO freq

	//GPIO
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK);//ON
	GPIO->DIRSET[0] = (1UL<<LED_USER1);//OUTPUT
	GPIO->CLR[0] = (1UL<<LED_USER1);//ON

	GPIO->DIRSET[0] = (1UL<<LED_USER2);//OUTPUT
	GPIO->CLR[0] = (1UL<<LED_USER2);//ON

	//MRT setup
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_MRT_MASK);//MRT ON

	//RESET
	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_MRT_RST_N_MASK);//RESET
	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_MRT_RST_N_MASK);//CLEAR BIT

	//chan0 and 1 into oneshot
	MRT0->CHANNEL[MRT_CHAN0].CTRL = (MRT_ONESHOT << MRT_CHANNEL_CTRL_MODE_SHIFT
			| MRT_CHANNEL_CTRL_INTEN_MASK);

	MRT0->CHANNEL[MRT_CHAN1].CTRL = (MRT_ONESHOT << MRT_CHANNEL_CTRL_MODE_SHIFT
				| MRT_CHANNEL_CTRL_INTEN_MASK);

	//START
	MRT0->CHANNEL[MRT_CHAN0].INTVAL = IVALUE_CH0 & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);
	MRT0->CHANNEL[MRT_CHAN1].INTVAL = IVALUE_CH1 & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

	//ENABLE INTERRUPTS
	NVIC_EnableIRQ(MRT0_IRQn);
	__enable_irq();

	while(1)
	{
		asm("NOP");
	}
	return 0;

}
