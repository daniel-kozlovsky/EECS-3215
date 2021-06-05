#include "clock_config.h"
#include "LPC802.h"

#include "seven_segment.h"

#define MRT_REPEAT (0)
#define MRT_GFLAG0 (0)

#define MRT_CHAN0 (0)
#define MRT_CHAN1 (1)

#define SEGMENT_PERIOD_TICKS (150000) //TWO IRQS IN COUNTER TOTAL

#define INIT_COUNT_IRQ_CH0 (SEGMENT_PERIOD_TICKS/2)
#define INIT_PWM_DUTY_FACTOR 0.95 //95% duty cycle. Bright but safe

#define DECIMAL_PERIOD (30000000)
#define INIT_COUNT_IRQ_CH1 (DECIMAL_PERIOD/2)
#define INIT_DECIMAL_PWM_DUTY_FACTOR 0.5
#define NUM_DIGITS 4

const uint32_t segmentCode[10] =
{
		(1UL<<SEG_A) | (1UL<<SEG_B) |(1UL<<SEG_C) |(1UL<<SEG_D) |(1UL<<SEG_E) |(1UL<<SEG_F), //0
		(1UL<<SEG_B) | (1UL<<SEG_C),														 //1
		(1UL<<SEG_A) | (1UL<<SEG_B) |(1UL<<SEG_D) |(1UL<<SEG_E) |(1UL<<SEG_G),               //2
		(1UL<<SEG_A) | (1UL<<SEG_B) |(1UL<<SEG_C) |(1UL<<SEG_D) |(1UL<<SEG_G),				 //3
		(1UL<<SEG_B) | (1UL<<SEG_C) |(1UL<<SEG_F) |(1UL<<SEG_G),							 //4
		(1UL<<SEG_A) | (1UL<<SEG_C) |(1UL<<SEG_D) |(1UL<<SEG_F) |(1UL<<SEG_G),				 //5
		(1UL<<SEG_A) | (1UL<<SEG_C) |(1UL<<SEG_D) |(1UL<<SEG_F) |(1UL<<SEG_G) |(1UL<<SEG_E), //6
		(1UL<<SEG_A) | (1UL<<SEG_B) |(1UL<<SEG_C),											 //7
		(1UL<<SEG_A) | (1UL<<SEG_B) |(1UL<<SEG_C) |(1UL<<SEG_D) |(1UL<<SEG_E) |(1UL<<SEG_F)|(1UL<<SEG_G), //8
		(1UL<<SEG_A) | (1UL<<SEG_B) |(1UL<<SEG_C) |(1UL<<SEG_D) |(1UL<<SEG_F) |(1UL<<SEG_G)  //9
};

const int digitCode[4] =
{
		1UL<<DIGIT_4, 1UL<<DIGIT_3, 1UL<<DIGIT_2,1UL<<DIGIT_1
};

const static int allSegments = segmentCode[8];// | (1UL<<DECIMAL);
const static int allDigits = (digitCode[0] |digitCode[1] |digitCode[2] |digitCode[3]);
static int digitsOfNum[NUM_DIGITS];

void MRT0_IRQHandler(void);
void startDisplayNumber(void);
void displayDigit(int, int);
void clearDisplay(void);
void initOutput(void);
int changeNumberToDisplay(int);
/**
 * returns -1 if number is larger than 4 digits
 */

void MRT0_IRQHandler(void)
{
	int static chan = 0;

	float static duty_factor = INIT_PWM_DUTY_FACTOR;
	float static decimal_duty_factor = INIT_DECIMAL_PWM_DUTY_FACTOR;

	int static pwm_state = 1; //led on
	int static decimal_pwm_state = 1;
	int static decimalFlag = 1;
	static int digit = 0;

	if(MRT0->IRQ_FLAG & (1<<MRT_GFLAG0))
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
		if(pwm_state == 1)
			{

				//high part of duty cycle
				switch(digit)
				{
				case 0:
					displayDigit(1, digitsOfNum[0]);//display first digit no matter what
					break;

				case 1:
					if(digitsOfNum[3] != 0 || digitsOfNum[2] != 0)//second digit only if one preceding zero
					{
						displayDigit(2, digitsOfNum[1]);
					}
					else if(digitsOfNum[1] != 0)//if second digit is not zero
					{
						displayDigit(2, digitsOfNum[1]);
					}
					break;

				case 2:
					//third digit only display if there is no preceding zero or if it is not zero itself
					if(digitsOfNum[3] != 0 || digitsOfNum[2] != 0)
					{
						displayDigit(3, digitsOfNum[2]);

					}

					break;

				case 3:
					//fourth digit only if it is not zero
					if(digitsOfNum[3] != 0)
					{
						displayDigit(4, digitsOfNum[3]);

					}

					break;
				case 4:
					//decimal
					if(decimalFlag)
					{
						//on
						GPIO->SET[0] = (1UL<<DIGIT_2);
						GPIO->CLR[0] = (1UL<<DECIMAL);
					}
					break;

				}

				MRT0->CHANNEL[MRT_CHAN0].INTVAL = (uint32_t)(INIT_COUNT_IRQ_CH0 * (1-duty_factor))
						& ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

				pwm_state = 0;
			}
			else
			{
				//low part of duty cycle
				clearDisplay();
				GPIO->SET[0] = (1UL<<DECIMAL);
				MRT0->CHANNEL[MRT_CHAN0].INTVAL = (uint32_t)(INIT_COUNT_IRQ_CH0 * (duty_factor))
								& ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

				pwm_state = 1;
				digit++;
				digit %= NUM_DIGITS + 1;//cycle between all four digits
			}
	}
	else
	{
		if(decimal_pwm_state == 1)
			{


				decimalFlag = 0;
				MRT0->CHANNEL[MRT_CHAN1].INTVAL = (uint32_t)(INIT_COUNT_IRQ_CH1 * (1-decimal_duty_factor))
						& ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

				decimal_pwm_state = 0;
			}
			else
			{
				decimalFlag = 1;
				MRT0->CHANNEL[MRT_CHAN1].INTVAL = (uint32_t)(INIT_COUNT_IRQ_CH1 * (decimal_duty_factor))
								& ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

				decimal_pwm_state = 1;
			}
	}

	return;
}
void startDisplayNumber()
{


	NVIC_DisableIRQ(MRT0_IRQn);

	//Main clock to 12Mhz

	SYSCON->MAINCLKSEL = (0x0<<SYSCON_MAINCLKSEL_SEL_SHIFT);

	SYSCON->MAINCLKUEN &= ~(0x1);
	SYSCON->MAINCLKUEN |= 0x1;

	//BOARD_BootClockFRO30M();//15Mhz FRO freq

	//MRT setup
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_MRT_MASK);//MRT ON

	//RESET
	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_MRT_RST_N_MASK);//RESET
	SYSCON->PRESETCTRL0 |= (SYSCON_PRESETCTRL0_MRT_RST_N_MASK);//CLEAR BIT

	//chan0 into oneshot
	MRT0->CHANNEL[MRT_CHAN0].CTRL = (MRT_REPEAT << MRT_CHANNEL_CTRL_MODE_SHIFT
			| MRT_CHANNEL_CTRL_INTEN_MASK);

	//chan1 for Decimal
	MRT0->CHANNEL[MRT_CHAN1].CTRL = (MRT_REPEAT << MRT_CHANNEL_CTRL_MODE_SHIFT
				| MRT_CHANNEL_CTRL_INTEN_MASK);

	//START
	MRT0->CHANNEL[MRT_CHAN0].INTVAL = INIT_COUNT_IRQ_CH0 & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);
	MRT0->CHANNEL[MRT_CHAN1].INTVAL = INIT_COUNT_IRQ_CH1 & ~(MRT_CHANNEL_INTVAL_LOAD_MASK);

	//ENABLE INTERRUPTS
	NVIC_EnableIRQ(MRT0_IRQn);
}

void displayDigit(int digit, int number)
{
	GPIO->SET[0] = digitCode[digit-1];
	GPIO->CLR[0] = segmentCode[number];
}
void clearDisplay()
{
	GPIO->CLR[0] = allDigits;
	GPIO->SET[0] = allSegments;
}

void initOutput()
{

	clearDisplay();
	//set all as output
	GPIO->DIRSET[0] = allSegments | (1ul<<DECIMAL);
	GPIO->DIRSET[0] = allDigits;
}

int changeNumberToDisplay(int num)
{
	if(num >= 10000)
	{
		return -1;
	}
	digitsOfNum[0] = num % 10;
	digitsOfNum[1] = (num/10) % 10;
	digitsOfNum[2] = (num/100) % 10;
	digitsOfNum[3] = (num/1000) % 10;
	return 0;
}


