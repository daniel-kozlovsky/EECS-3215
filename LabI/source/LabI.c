/*
#include "LPC802.h"
#include <stdint.h>
	TODO:	insert	other	definitions	and	declarations	here.
void init_ADC(void);

//	The	Potentiometer	on	the	LPC	board	is	connected	to	PIO0_15	(ADC_8)
//	Set	PINENABLE0	register	bit	18	to	0	to	make	PIO0_15	to	ADC_8	(Table	97	in	User	Manual)
//	Bits	10	17	and	19	to	21	to	1	to	disable	the	other	channels	(they	are	1	out	of	reset)
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

int main(void)	{
	int i	=	0;
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
		Step	4.	Enter	an	infinite	loop,	continuously	sample	Ch	8	on	the	ADC.
	while(1)	{
		//	Step	6.	read	result	bits	in	DAT8	(data	for	channel	8)	for	conversion	result.
		ADC0->SEQ_CTRL[0]	&=	~(1UL<<ADC_SEQ_CTRL_START_SHIFT);		//	start	bit	to	0
		ADC0->SEQ_CTRL[0]	|=	(1UL<<ADC_SEQ_CTRL_START_SHIFT);		//	start	bit	to	1
		//	Read	the	captured	value	on	ADC	ch	8.	Assign	it	to	a	variable.
		adc_result	=	((ADC0->DAT[8])&(ADC_DAT_RESULT_MASK));//	isolate	bits	15:4	(data)
		adc_result	=	(adc_result>>ADC_DAT_RESULT_SHIFT);//	shift	right;get	true	numeric	value.
	}
	return 0;
}
*/
