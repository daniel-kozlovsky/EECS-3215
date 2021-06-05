#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "LPC802.h"
#include "fsl_debug_console.h"
#include <stdlib.h>

#include "fsl_iap.h"
#include "seven_segment.h"

#define TAMPER_BTN 8

#define WKT_FREQ 1000000 //1MHz (LPO)
#define WKT_INT_FREQ (10) // interrupt frequency of WKT in seconds (30) //TODO change to 30 after testing
#define WKT_RELOAD (WKT_FREQ * WKT_INT_FREQ) // Reload value for the WKT down counter

void SysTick_Handler(void);
void initWKT(void);
int getNewTOTP(void);
void tamper(void);
void PIN_INT0_IRQHandler(void);

int main(void) {

	__disable_irq();
	//GPIO Setup
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK |
								SYSCON_SYSAHBCLKCTRL0_GPIO_INT_MASK);

	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK |
							SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);

	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK|
							SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);
	//input
	NVIC_DisableIRQ(PIN_INT0_IRQn);

	GPIO->DIRCLR[0] = (1UL<<TAMPER_BTN);
	SYSCON->PINTSEL[0] = TAMPER_BTN;

	PINT->ISEL = 0x00;

	//Falling edge interrupt
	PINT->CIENR = 0b00000001;
	PINT->SIENF = 0b00000001;

	PINT->IST = 0xFF;

	NVIC_EnableIRQ(PIN_INT0_IRQn);


	time_t t;
	srand((unsigned) time(&t));

	initOutput();
	changeNumberToDisplay(getNewTOTP());
	initWKT();
	startDisplayNumber();

	__enable_irq();

	while(1)
	{
		__asm("nop");
	}
	return 0;
}

void WKT_IRQHandler(void)
{
	//clear interrupt flag
	WKT->CTRL = WKT_CTRL_ALARMFLAG_MASK;

	//reload WKT
	WKT->COUNT = WKT_RELOAD;

	//calculate new value and display
	changeNumberToDisplay(getNewTOTP());
}

void initWKT()
{
	NVIC_DisableIRQ(WKT_IRQn);

	//Clock WKT
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_WKT_MASK);

	//Low power oscillator power on
	SYSCON->PDRUNCFG &= ~(SYSCON_PDRUNCFG_LPOSC_PD_MASK);

	//enable lpo to WKT
	SYSCON->LPOSCCLKEN |= (SYSCON_LPOSCCLKEN_WKT_MASK);

	//Reset WKT
	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_WKT_RST_N_MASK);
	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_WKT_RST_N_MASK);

	//Set WKT to use the lpo
	WKT->CTRL = WKT_CTRL_CLKSEL_MASK;

	//Load counter
	WKT->COUNT = WKT_RELOAD;

	//enable interrupts WKT
	NVIC_EnableIRQ(WKT_IRQn);

}

int getNewTOTP()
{
	return rand() % 10000;
}

void tamper()
{
	/* Board pin, clock, debug console init */
	    /* Attach 12 MHz clock to USART0 (debug console) */
	    CLOCK_Select(kUART0_Clk_From_MainClk);

//	    BOARD_InitBootPins();
//	    BOARD_InitBootClocks();
//	    BOARD_InitDebugConsole();

	    /* Erase sector before writing */
	    IAP_PrepareSectorForWrite(0, 15);
	    IAP_EraseSector(0, 15, SystemCoreClock);
}

void PIN_INT0_IRQHandler(void)
{
	if(PINT->IST & (1<<0))
	{
		PINT->IST = (1<<0);

		tamper();
	}
	else
	{
		asm("NOP");
	}
}








