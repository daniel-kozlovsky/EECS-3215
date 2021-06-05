
#include "clock_config.h"
#include "LPC802.h"

#define  DIGIT_1 17
#define  DIGIT_2 11
#define  DIGIT_3 14//sclk
#define  DIGIT_4 10

#define SEG_A 4
#define SEG_B 8
#define SEG_C 13
#define SEG_D 9
#define SEG_E 7
#define SEG_F 12
#define SEG_G 1

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

int main(void) {
	//enable gpio

	SYSCON->SYSAHBCLKCTRL0 |= SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK;

	SYSCON->PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK);

	SYSCON->PRESETCTRL0 |= SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK;

	//set output
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



	clearDisplay();

	//4879
	//digit 1 :4
	GPIO->SET[0] = (1UL<<DIGIT_1);//turn on digit 1
	GPIO->CLR[0] = (1UL<<DIGIT_2 | 1UL<<DIGIT_3 | 1UL<<DIGIT_4);//turn off all others

	GPIO->CLR[0] = (1UL<<SEG_B |1UL<<SEG_C | 1UL<<SEG_F |1UL<<SEG_G);

	//digit 2 :8
	clearDisplay();
	GPIO->SET[0] = (1UL<<DIGIT_2);//turn on digit 2
	GPIO->CLR[0] = (1UL<<DIGIT_1 | 1UL<<DIGIT_3 | 1UL<<DIGIT_4);//turn off all others

	GPIO->CLR[0] = (1UL<<SEG_A |1UL<<SEG_B | 1UL<<SEG_C | 1UL<<SEG_D | 1UL<<SEG_E |1UL<<SEG_F | 1UL<<SEG_G);//8

	//4879
	//digit 3 :7
	clearDisplay();
	GPIO->SET[0] = (1UL<<DIGIT_3);//turn on digit 3
	GPIO->CLR[0] = (1UL<<DIGIT_2 | 1UL<<DIGIT_1 | 1UL<<DIGIT_4);//turn off all others

	GPIO->CLR[0] = (1UL<<SEG_A |1UL<<SEG_B | 1UL<<SEG_C);

	//4879
	//digit 4 :9
	clearDisplay();
	GPIO->SET[0] = (1UL<<DIGIT_4);//turn on digit 4
	GPIO->CLR[0] = (1UL<<DIGIT_2 | 1UL<<DIGIT_3 | 1UL<<DIGIT_1);//turn off all others

	GPIO->CLR[0] = (1UL<<SEG_A |1UL<<SEG_B | 1UL<<SEG_C | 1UL<<SEG_D | 1UL<<SEG_F | 1UL<<SEG_G);//8

    while(1) {

    	//4879
		//digit 1 :4
    	clearDisplay();
		GPIO->SET[0] = (1UL<<DIGIT_1);//turn on digit 1
		GPIO->CLR[0] = (1UL<<DIGIT_2 | 1UL<<DIGIT_3 | 1UL<<DIGIT_4);//turn off all others

		GPIO->CLR[0] = (1UL<<SEG_B |1UL<<SEG_C | 1UL<<SEG_F |1UL<<SEG_G);

		//digit 2 :8
		clearDisplay();
		GPIO->SET[0] = (1UL<<DIGIT_2);//turn on digit 2
		GPIO->CLR[0] = (1UL<<DIGIT_1 | 1UL<<DIGIT_3 | 1UL<<DIGIT_4);//turn off all others

		GPIO->CLR[0] = (1UL<<SEG_A |1UL<<SEG_B | 1UL<<SEG_C | 1UL<<SEG_D | 1UL<<SEG_E |1UL<<SEG_F | 1UL<<SEG_G);//8

		//4879
		//digit 3 :7
		clearDisplay();
		GPIO->SET[0] = (1UL<<DIGIT_3);//turn on digit 3
		GPIO->CLR[0] = (1UL<<DIGIT_2 | 1UL<<DIGIT_1 | 1UL<<DIGIT_4);//turn off all others

		GPIO->CLR[0] = (1UL<<SEG_A |1UL<<SEG_B | 1UL<<SEG_C);

		//4879
		//digit 4 :9
		clearDisplay();
		GPIO->SET[0] = (1UL<<DIGIT_4);//turn on digit 4
		GPIO->CLR[0] = (1UL<<DIGIT_2 | 1UL<<DIGIT_3 | 1UL<<DIGIT_1);//turn off all others

		GPIO->CLR[0] = (1UL<<SEG_A |1UL<<SEG_B | 1UL<<SEG_C | 1UL<<SEG_D | 1UL<<SEG_F | 1UL<<SEG_G);//8
        __asm volatile ("nop");
    }
    return 0 ;
}
