#ifndef SEVEN_SEGMENT_H_
#define SEVEN_SEGMENT_H_



#endif /* SEVEN_SEGMENT_H_ */

#define  DIGIT_1 17
#define  DIGIT_2 11
#define  DIGIT_3 14
#define  DIGIT_4 10

#define DECIMAL 16 //8

//pin assignments for the display
#define SEG_A 4
#define SEG_B 0
#define SEG_C 13
#define SEG_D 9
#define SEG_E 7
#define SEG_F 12
#define SEG_G 1

void MRT0_IRQHandler(void);
void startDisplayNumber(void);
void displayDigit(int, int);
void clearDisplay(void);
void initOutput(void);
int changeNumberToDisplay(int);
