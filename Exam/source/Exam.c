
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "LPC802.h"
#include "fsl_debug_console.h"

#define LED1 9
#define LED2 12
#define BUTTON_USER 8

#define MRT_ONESHOT (1)
#define DELAY (60000000) // (counts) irq desired period --5 sec

#define WKT_FREQ 1000000 //1MHz (LPO)
#define WKT_INT_FREQ (0.33) // interrupt frequency of WKT in seconds
#define WKT_RELOAD (WKT_FREQ * WKT_INT_FREQ) // Reload value for the WKT down counter

#define SEGMENT_PERIOD_TICKS (120000) //TWO IRQS IN COUNTER TOTAL
#define MRT_REPEAT (0)
#define INIT_COUNT_IRQ_CH0 (SEGMENT_PERIOD_TICKS/2)
#define INIT_PWM_DUTY_FACTOR 0.95 //95% duty cycle. Bright but safe

//7 segment pinouts for a Broadcom HDSP-B04E based on lab H
#define  DIGIT_1 12//17
#define  DIGIT_2 9//11
#define  DIGIT_3 8//14//sclk
#define  DIGIT_4 6//10

#define SEG_A 11/4
#define SEG_B 7//0
#define SEG_C 4//13
#define SEG_D 2//9
#define SEG_E 1//7
#define SEG_F 10//12
#define SEG_G 5//1
//
////my 7 segment pin map
//#define  DIGIT_1 17
//#define  DIGIT_2 11
//#define  DIGIT_3 14
//#define  DIGIT_4 10
//
//#define DECIMAL 16 //8
//
////pin assignments for the display
//#define SEG_A 4
//#define SEG_B 0
//#define SEG_C 13
//#define SEG_D 9
//#define SEG_E 7
//#define SEG_F 12
//#define SEG_G 1

void task1();
void SysTick_Handler(void);
void task2();
void WKT_IRQHandler(void);
void task3a(void);
void clearDisplay(void);
void task4();
void PIN_INT0_IRQHandler(void);
void flashLEDThreeTimes();
void task5();
void init_ADC(void);

static int flashFlag = 0;
static int flashCount = 0;

static int taskNum;
int main(void) {

	//MRT for tasks 5 seconds apart
	__disable_irq();//global
	NVIC_DisableIRQ(MRT0_IRQn); //MRT0 interrupt off

	//GPIO Setup with interrupts
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK |
									SYSCON_SYSAHBCLKCTRL0_GPIO_INT_MASK |
									SYSCON_SYSAHBCLKCTRL0_MRT_MASK);

	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK |
								SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK|
								SYSCON_PRESETCTRL0_MRT_RST_N_MASK);

	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK|
								SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK|
								SYSCON_PRESETCTRL0_MRT_RST_N_MASK);

	//Set up main clock
	SYSCON->MAINCLKSEL = (0x0<<SYSCON_MAINCLKSEL_SEL_SHIFT);
	SYSCON->MAINCLKUEN &= ~(0x1);
	SYSCON->MAINCLKUEN |= 0X1;

	BOARD_BootClockFRO24M(); //12 Mhz fRO for systick

	GPIO->DIRSET[0] = (1UL<<LED1);//OUTPUT
	GPIO->DIRSET[0] = (1UL<<LED2);



	//Output for 7 segment
	GPIO->DIRSET[0] = (1UL<<DIGIT_1);
	GPIO->DIRSET[0] = (1UL<<DIGIT_2);
	GPIO->DIRSET[0] = (1UL<<DIGIT_3);
	GPIO->DIRSET[0] = (1UL<<DIGIT_4);
	GPIO->DIRSET[0] = (1UL<<SEG_A);
	GPIO->DIRSET[0] = (1UL<<SEG_B);
	GPIO->DIRSET[0] = (1UL<<SEG_C);
	GPIO->DIRSET[0] = (1UL<<SEG_D);
	GPIO->DIRSET[0] = (1UL<<SEG_E);
	GPIO->DIRSET[0] = (1UL<<SEG_F);
	GPIO->DIRSET[0] = (1UL<<SEG_G);

	GPIO->SET[0] = (1UL<<LED1);//Off
	GPIO->SET[0] = (1UL<<LED2);//Off
	clearDisplay();

	//MRT one shot for 5 second delays
	MRT0->CHANNEL[0].CTRL = (MRT_ONESHOT << MRT_CHANNEL_CTRL_MODE_SHIFT
				| MRT_CHANNEL_CTRL_INTEN_MASK);

	//chan 1 for segment display
	MRT0->CHANNEL[1].CTRL = (MRT_REPEAT << MRT_CHANNEL_CTRL_MODE_SHIFT
					| MRT_CHANNEL_CTRL_INTEN_MASK);

	//start timer (5 sec)
	MRT0->CHANNEL[0].INTVAL = DELAY & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);
	taskNum = 1;
	NVIC_EnableIRQ(MRT0_IRQn);
	__enable_irq();


	return 0 ;
}

//handler for MRT interrupts
void MRT0_IRQHandler(void)
{

	int static chan = 0;

	float static duty_factor = INIT_PWM_DUTY_FACTOR;

	int static pwm_state = 1; //led on
	static int digit = 0;

	if(MRT0->IRQ_FLAG & (1<<0))
	{
		chan = 0;
	}
	else
	{
		chan = 1;
	}

	MRT0->CHANNEL[chan].STAT = MRT_CHANNEL_STAT_INTFLAG_MASK;//remove the interrupt flag

	if(chan == 0)
	{


		switch (taskNum)
				{
				case 1:
					task1();
					MRT0->CHANNEL[0].INTVAL = DELAY & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);
					break;
				case 2:
					SysTick->CTRL = ~SysTick_CTRL_ENABLE_Msk;
					task2();
					MRT0->CHANNEL[0].INTVAL = DELAY & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);
					break;
				case 3:
					task3a();
					MRT0->CHANNEL[0].INTVAL = DELAY & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);
					break;
				case 4:
					MRT0->CHANNEL[1].INTVAL = MRT_CHANNEL_INTVAL_LOAD_MASK;
					task4();
					MRT0->CHANNEL[0].INTVAL = DELAY & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);
					break;
				case 5:
					task5();//blocking

					break;
				}
		taskNum++;

	}



	else if(chan == 1)
		{
			if(pwm_state == 1)
				{

					//high part of duty cycle
					switch(digit)
					{
					case 0:
						GPIO->SET[0] = (1UL<<DIGIT_4);//turn on digit 4

						GPIO->CLR[0] = (1UL<<SEG_C |1UL<<SEG_D | 1UL<<SEG_E | 1UL<<SEG_F | 1UL<<SEG_G);//b
						break;

					case 1:
						GPIO->SET[0] = (1UL<<DIGIT_3);//turn on digit 3

						GPIO->CLR[0] = (1UL<<SEG_A | 1UL<<SEG_B | 1UL<<SEG_C | 1UL<<SEG_E | 1UL<<SEG_F | 1UL<<SEG_G);//A
						break;

					case 2:
						GPIO->SET[0] = (1UL<<DIGIT_2);//turn on digit 2

						GPIO->CLR[0] = (1UL<<SEG_A | 1UL<<SEG_E | 1UL<<SEG_F | 1UL<<SEG_G);//F
						break;

					case 3:
						GPIO->SET[0] = (1UL<<DIGIT_1);//turn on digit 1

								GPIO->CLR[0] = (1UL<<SEG_A | 1UL<<SEG_E | 1UL<<SEG_F | 1UL<<SEG_G);//F
						break;
					}

					MRT0->CHANNEL[1].INTVAL = (uint32_t)(INIT_COUNT_IRQ_CH0 * (1-duty_factor))
							& ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

					pwm_state = 0;
				}
				else
				{
					//low part of duty cycle
					clearDisplay();
					GPIO->SET[0] = (1UL<<DECIMAL);
					MRT0->CHANNEL[1].INTVAL = (uint32_t)(INIT_COUNT_IRQ_CH0 * (duty_factor))
									& ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

					pwm_state = 1;
					digit++;
					digit %= 5;//cycle between all four digits
				}
		}
}
//Task 1 flash LED 1 @ 3Hz using SysTick
void task1()
{
	NVIC_DisableIRQ(SysTick_IRQn);

	SysTick_Config(4000000);//3hz flash

	flashFlag = 0;

	NVIC_EnableIRQ(SysTick_IRQn);
}

void SysTick_Handler(void)
{
	//for task 4
	if(flashFlag)
	{
		if(flashCount == 6)
		{
			//disable timer
			SysTick->CTRL = ~SysTick_CTRL_ENABLE_Msk;
			flashCount = 0;
		}
		else
		{
			GPIO->NOT[0] = (1UL<<LED2);
			flashCount++;
		}

	}
	else//for task 1
	{
		GPIO->NOT[0] = (1UL<<LED1);
	}

}

//Task 1 flash LED 2 @ 3Hz using WKT
void task2()
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

void WKT_IRQHandler(void)
{
	//clear interrupt flag
	WKT->CTRL = WKT_CTRL_ALARMFLAG_MASK;

	GPIO->NOT[0] = (1UL<<LED2);//Flash LED2

	//reload WKT
	WKT->COUNT = WKT_RELOAD;

}

//Display FFAB on the seven segment display 3a
void task3a()
{
	clearDisplay();
	//start display
	MRT0->CHANNEL[1].INTVAL = INIT_COUNT_IRQ_CH0 & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

//	while(taskNum == 3)
//	{
//		//Digit 1 : b
//		clearDisplay();
//		GPIO->SET[0] = (1UL<<DIGIT_4);//turn on digit 4
//
//		GPIO->CLR[0] = (1UL<<SEG_C |1UL<<SEG_D | 1UL<<SEG_E | 1UL<<SEG_F | 1UL<<SEG_G);//b
//
//		//Digit 2: A
//		clearDisplay();
//		GPIO->SET[0] = (1UL<<DIGIT_3);//turn on digit 3
//
//		GPIO->CLR[0] = (1UL<<SEG_A | 1UL<<SEG_B | 1UL<<SEG_C | 1UL<<SEG_E | 1UL<<SEG_F | 1UL<<SEG_G);//A
//
//
//		//Digit 3: F
//		clearDisplay();
//		GPIO->SET[0] = (1UL<<DIGIT_2);//turn on digit 2
//
//		GPIO->CLR[0] = (1UL<<SEG_A | 1UL<<SEG_E | 1UL<<SEG_F | 1UL<<SEG_G);//F
//
//		//Digit 4: F
//		clearDisplay();
//		GPIO->SET[0] = (1UL<<DIGIT_1);//turn on digit 1
//
//		GPIO->CLR[0] = (1UL<<SEG_A | 1UL<<SEG_E | 1UL<<SEG_F | 1UL<<SEG_G);//F
//	}
//	clearDisplay();
}

void clearDisplay()
{
	GPIO->CLR[0] = (1UL<<DIGIT_1 | 1UL<<DIGIT_2 | 1UL<<DIGIT_3 | 1UL<<DIGIT_4);
	GPIO->SET[0] = (1UL<<SEG_A);
	GPIO->SET[0] = (1UL<<SEG_B);
	GPIO->SET[0] = (1UL<<SEG_C);
	GPIO->SET[0] = (1UL<<SEG_D);
	GPIO->SET[0] = (1UL<<SEG_E);
	GPIO->SET[0] = (1UL<<SEG_F);
	GPIO->SET[0] = (1UL<<SEG_G);
}
//Task 4 flash LED2 3 times if button is pressed;else do nothing
void task4()
{
	NVIC_DisableIRQ(PIN_INT0_IRQn);

	//input for button
	GPIO->DIRCLR[0] = (1UL<<BUTTON_USER);

	//Button interrupt setup
	SYSCON->PINTSEL[0] = BUTTON_USER;
	PINT->ISEL = 0x00;
	//Falling edge interrupt
	PINT->CIENR = 0b00000001;
	PINT->SIENF = 0b00000001;
	//reset interrupt
	PINT->IST = 0xFF;
	GPIO->SET[0] = (1UL<<LED1);//Off
	GPIO->SET[0] = (1UL<<LED2);//Off
	flashFlag = 1;
	NVIC_EnableIRQ(PIN_INT0_IRQn);

}

void PIN_INT0_IRQHandler(void)
{
	//make sure its the right interrupt
	if(PINT->IST & (1<<0))
	{
		//clear flag
		PINT->IST = (1<<0);

		//flash three times
		flashLEDThreeTimes();
	}
	else
	{
		asm("NOP");
	}
}

void flashLEDThreeTimes()
{

	SysTick_Config(6000000);//2hz flash
	flashFlag = 1;
}

//turn pot 20d cw, flash led, turn pot back to centre, flash led
//Some of the code is taken from LAB I
void task5()
{
	clearDisplay();
	int stepCounter = 0;
	flashFlag = 1;
	uint32_t volatile adc_result	=	0;
	//	Enable	ADC	and,	in	particular	ADC	Ch.	8	to	the	potentiometer.
	init_ADC();
	//	Step	1.	Select	ADC	channel	8	via	CHANNELS	bits	in	SEQA_CTL.
	// ADC0->SEQ_CTRL[0]	is	SEQA
	//	ADC0->SEQ_CTRL[1]	is	SEQB.
	//	The	channel	bits	are	the	bottom	12	bits:	i.e.		bits	11	to	0.
	//	A	1	in	any	bit	in	the	bottom	12	bits	will	turn	THAT	channel	on:
	//	ADC_0	is	bit	0	of	ADC0->SEQ_CTRL[0]	(SEQA)
	//	ADC_8	is	bit	8	of	ADC0->SEQ_CTRL[0]	(SEQA)
	ADC0->SEQ_CTRL[0]	|=	(1UL<<8);//	turn	on	ADC	channel	8.
	//	Step	2.	Set	TRIGPOL	to	1	and	SEQ_ENA	to	1	in	SEQA_CTRL	register
	ADC0->SEQ_CTRL[0]	|=	(1UL<<ADC_SEQ_CTRL_TRIGPOL_SHIFT);	//	trig	pol	set	to	1.
	ADC0->SEQ_CTRL[0]	|=	(1UL<<ADC_SEQ_CTRL_SEQ_ENA_SHIFT);	//	Sequence	A	turned	ON.
	//	Step	3.	set	START	bit	to	1	in	SEQA_CTRL	register
	//	This	bit	can	only	be	set	momentarily.		It	immediately	goes	back	to 0
	//	and	so	always	gets	read	back	as	0.
	ADC0->SEQ_CTRL[0]	|=	(1UL<<ADC_SEQ_CTRL_START_SHIFT);		//	start	bit	to	1
	/*	Step	4.	Enter	an	infinite	loop,	continuously	sample	Ch	8	on	the	ADC.*/
	while(1)	{
		//	Step	6.	read	result	bits	in	DAT8	(data	for	channel	8)	for	conversion	result.
		ADC0->SEQ_CTRL[0]	&=	~(1UL<<ADC_SEQ_CTRL_START_SHIFT);		//	start	bit	to	0
		ADC0->SEQ_CTRL[0]	|=	(1UL<<ADC_SEQ_CTRL_START_SHIFT);		//	start	bit	to	1
		//	Read	the	captured	value	on	ADC	ch	8.	Assign	it	to	a	variable.
		adc_result	=	((ADC0->DAT[8])&(ADC_DAT_RESULT_MASK));//	isolate	bits	15:4	(data)
		adc_result	=	(adc_result>>ADC_DAT_RESULT_SHIFT);//	shift	right;get	true	numeric	value.
		//20d : <1700; 0d: >2100
		if(stepCounter == 0 && adc_result <1700)
		{
			flashLEDThreeTimes();
			stepCounter++;
		}
		else if(stepCounter == 1 && adc_result > 2050)
		{
			flashLEDThreeTimes();
			stepCounter++;
		}
	}
}

//Taken From LAB I
void init_ADC(void)
{
	//	---------------------------------------------------
	//	Step	1.	Power	the	ADC	with	PDRUNCFG
	//	Power	Config,	Bit	4:	0	is	powered	on,	1	is	powered	down.
	SYSCON->PDRUNCFG &=	~(SYSCON_PDRUNCFG_ADC_PD_MASK);
	//	---------------------------------------------------
	//	Step	2.	SYSAHBCLKCTRL	to	enable	clock	register	interfaces:
	//	1.	ADC	register	interface	ON
	//	2.	Switch	Matrix	register	interface	ON.
	//	See	Table	63	in	the	User	Manual
	//	Here,	turn	on	ADC	and	SWM
	SYSCON->SYSAHBCLKCTRL0 |=	( SYSCON_SYSAHBCLKCTRL0_ADC_MASK	|
	SYSCON_SYSAHBCLKCTRL0_SWM_MASK);
	//	Reset	the	ADC	module.
	//	Table	64	in	User	Manual.
	//	Bit	24	in	Peripheral	Reset	Control	Register	0:	go	0,	then	1	to	reset	and	clear.
	SYSCON->PRESETCTRL0 &=	~(SYSCON_PRESETCTRL0_ADC_RST_N_MASK);//	Assert reset	(0)
	SYSCON->PRESETCTRL0 |=	(SYSCON_PRESETCTRL0_ADC_RST_N_MASK);//	Remove	reset(1)
	//	---------------------------------------------------
	//	Step	3.	Use	ADCASYNCCLKSEL	and	ADCASYNCDIV	to	control	ADC	clock
	//	Use	the	FRO	(Free-running	Oscillator)	as	the	source	for	sync'ing	the	ADC	captures.
	//	The	ADC	Clock	Select	is	a	pair	of	bits	(SYSCON_ADCCLKSEL_SEL_MASK)
	//	Set	to	00 to	use	the	FRO	(yes!);	01	to	use	an	external	clock	(no!);	11	is	no	clock.	(no!)
	//	FRO	runs	at	750	kHz.
	SYSCON->ADCCLKSEL &=	~(SYSCON_ADCCLKSEL_SEL_MASK);	//	Use	fro_clk	as	source	for	ADC	async clock
	//	Divide	the	FRO	clock	into	the	ADC.		If	0	it	shuts	down	the	ADC	clock?
	SYSCON->ADCCLKDIV =	1;//	divide	by	1	(values:	0	to	255)
	//	---------------------------------------------------
	//	Step	4.	do	you	deal	with	one	of	the	four	IRQs	for	the	ADC?
	//	i.e.	ADC_SEQA_IRQ,	ADC_SEQB_IRQ,	ADC_THCMP_IRQ,	ADC_OVR_IRQ
	//	For	now,	no.
	//	---------------------------------------------------
	//	Step	5.	Enable	a	particular	ADC	channel
	//	The	Potentiometer	on	the	LPC	board	is	connected	to	PIO0_15
	//	Set	PINENABLE0	register	bit	18	to	0	to	make	PIO0_15	to	ADC_8	(Table	97	in	User	Manual)
	//	Bits	10	17	and	19	to	21	to	1	to	disable	the	other	channels	(they	are	1	out	of	reset)
	//	Make	bit	18	a	0	(active	low)	to	turn	on	this	ADC	channel
	SWM0->PINENABLE0 &=	~(SWM_PINENABLE0_ADC_8_MASK);	//
}
