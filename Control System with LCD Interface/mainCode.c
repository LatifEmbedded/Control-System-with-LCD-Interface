/*
	CREATOR : DALI FETHI ABDELLATIF
	DATE : 30/05/2024
	PLATFORM : PIC18F452 + C18 MICROCHIP COMPILER



		The code is a complete program for a PIC18F452 microcontroller, written in C using the MPLAB C18 compiler. 
		The program involves controlling LEDs based on user input from a potentiometer and a push-button. 
		The LCD display provides a menu, and the potentiometer allows the user to select different tasks
		which are executed when the push-button is pressed. The tasks are:

																			1.Rotate LED
																			2.Shift LED
																			3.Blink LED
																			4.Swap LED
*/


#include <p18f452.h>
#pragma config WDT = OFF

#define rs PORTCbits.RC0
#define rw PORTCbits.RC1
#define en PORTCbits.RC2

unsigned char position = 0;
unsigned int newValue = 0;
unsigned int oldValue = 0;
unsigned char val = 0;

#pragma idata access state = 0x00
near unsigned char state = 0;

void delay250ms(void);					// DELAY FUNCTION 0.25 SECONDS
void initiaLcd(void);					// INITIALIZATION OF LCD
void delay3us(void);					// DELAY FUNCTION 3U SECONDS
void commandInst(void);					// COMMAND INSTRUCTION FUNCTION	
void busyFlag(void);					// BUSYFLAG FUNCTION
void dataInst(void);					// DATA FUNCTION
void timeAcqui(void);					// TIME ACQUIZITION FUNCTION
void changeLine(unsigned int value);	// CHANGE LINE
void display(void);						// DISPLAY FUNCTION
void shiftLeft(void);					// SHIFT LEFT THE CURSOR
void arrowFunction(void);				// DISPLAY THE ARROW
void clearPreviousFlesh(void);			// CLEAR THE PREVIOUS ARROW WHEN WE JUMP TO CHANGE LINE		
void swapLed(void);						// FUNCTION TO SWAP LED
void shiftLed(void);					// FUNCTION TO SHIFT LED
void blinkLed(void);					// FUNCTION TO BLINK LED
void rotateLed(void);					// FUNCTION TO ROTATE LED FROM RE0 TO RE2 AND RE2 TO RE0


#pragma interrupt function				// HIGH PRIORITY INTERRUPT SERVICE ROUTINE
void function(void)
{
	if(INTCONbits.INT0IF == 1)
	{
		INTCONbits.INT0IF = 0;
		switch(val)
		{
			case 0 :
				rotateLed();
				break;
			case 1:
				shiftLed();
				break;
			case 2 :
				blinkLed();
				break;
			case 3:
				swapLed();
				break;
		}
	}
	if(PIR1bits.ADIF == 1)	
	{
		PIR1bits.ADIF = 0;
		if(state == 0)
		{	
			oldValue = ADRESH;
			oldValue <<= 8;
			oldValue += ADRESL;
			changeLine(oldValue);	
			state = 1;
		}
		else
		{
			newValue = ADRESH;
			newValue <<= 8;
			newValue += ADRESL;
			if(newValue != oldValue)
			{		
				oldValue = newValue;
				changeLine(newValue);
			}			
		}
		timeAcqui();
		ADCON0bits.GO = 1;
	}
}

#pragma code myfunction = 0x00008			// INTERRUPT VECTOR TABLE HIGH PRIORITY
void myfunction(void)
{	
	_asm
		GOTO function
	_endasm
}
#pragma code

void main(void)
{
	TRISD = 0x00;		
	TRISC = 0xF8;
	TRISE = 0x00;
	TRISAbits.TRISA0 = 1;
	TRISBbits.TRISB0 = 1;
	initiaLcd();
	display();
	ADCON0 = 0x41;					// CONFIGURING THE ADC 
	ADCON1 = 0x8E;
	INTCONbits.GIE = 1;				// ENABLE GLOBAL INTERRUPT BIT
	INTCONbits.INT0IE = 1;			// ENABLE INT0 INTERRUPT
	INTCONbits.INT0IF = 0;
	INTCONbits.PEIE = 1;
	PIE1bits.ADIE = 1;				// ENABLE ADC INTERRUPT
	PIR1bits.ADIF = 0;
	timeAcqui();
	ADCON0bits.GO = 1;	
	while(1);
}
void initiaLcd(void)
{
	LATD = 0x38;
	commandInst();
	delay250ms();
	LATD = 0x01;
	commandInst();
	delay250ms();
	LATD = 0x0C;
	commandInst();
	delay250ms();
}
void delay250ms(void)
{
	T0CON = 0x01;
	TMR0H = 0x0B;
	TMR0L = 0xBC;
	INTCONbits.TMR0IF = 0;
	T0CONbits.TMR0ON = 1;
	while(INTCONbits.TMR0IF == 0);
	INTCONbits.TMR0IF = 0;
	T0CONbits.TMR0ON = 0;	
}
void delay3us(void)
{
	T0CON = 0x48;
	TMR0L = 253;
	INTCONbits.TMR0IF = 0;
	T0CONbits.TMR0ON = 1;
	while(INTCONbits.TMR0IF == 0);
	INTCONbits.TMR0IF = 0;
	T0CONbits.TMR0ON = 0;	
}
void commandInst(void)
{
	rs = 0;
	rw = 0;
	en = 1;
	delay3us();
	en = 0;
}
void dataInst(void)
{
	rs = 1;
	rw = 0;
	en = 1;
	delay3us();
	en = 0;
}
void busyFlag(void)
{
	rs = 0;
	rw = 1;	
	TRISDbits.TRISD7 = 1;
	do
	{
		en = 0;
		delay3us();
		en = 1;
	}while(PORTDbits.RD7 == 1);
	en = 0;
	TRISDbits.TRISD7 = 0;
}
void timeAcqui(void)
{
	T0CON = 0x48;
	TMR0L = 241;
	INTCONbits.TMR0IF = 0;
	T0CONbits.TMR0ON = 1;
	while(INTCONbits.TMR0IF == 0);
	INTCONbits.TMR0IF = 0;
	T0CONbits.TMR0ON = 0;	
}
void changeLine(unsigned int value)
{	
	static unsigned char state_1 = 0;
	val = (3*value)/1023;
	if(state_1 == 0)
		state_1 = 1;
	else
		clearPreviousFlesh();
	switch(val)
	{
		case 0 :
			LATD = 0x80;
			position = LATD;
			commandInst();
			busyFlag();
			arrowFunction();
			break;
		case 1 :
			LATD = 0xC0;
			position = LATD;
			commandInst();
			busyFlag();
			arrowFunction();
			break;
		case 2 :
			LATD = 0x90;
			position = LATD;
			commandInst();
			busyFlag();
			arrowFunction();
			break;
		case 3 :
			LATD = 0xD0;
			position = LATD;
			commandInst();
			busyFlag();
			arrowFunction();
			break;
	}
}
void display(void)
{
	unsigned char string_1[] = "1.ROTATE LED", i = 0, string_2[] = "2.SHIFT LED", string_3[] = "4.SWAP LED", string_4[] = "3.BLINK LED";
	shiftLeft();
	while(string_1[i] != '\0')
	{
		LATD = string_1[i];
		dataInst();
		busyFlag();
		++i;
	}
	i = 0;
	LATD = 0xC0;
	commandInst();
	busyFlag();
	shiftLeft();
	while(string_2[i] != '\0')
	{
		LATD = string_2[i];
		dataInst();
		busyFlag();
		++i;
	}
	i = 0;
	LATD = 0x90;
	commandInst();
	busyFlag();
	shiftLeft();
	while(string_4[i] != '\0')
	{
		LATD = string_4[i];
		dataInst();
		busyFlag();
		++i;
	}
	i = 0;
	LATD = 0xD0;
	commandInst();
	busyFlag();
	shiftLeft();
	while(string_3[i] != '\0')
	{
		LATD = string_3[i];
		dataInst();
		busyFlag();
		++i;
	}
}
void shiftLeft(void)
{
	unsigned char i = 3;
	while(i>0)	
	{
		LATD = 0x14;
		commandInst();
		busyFlag();
		--i;
	}
}
void arrowFunction(void)
{
	LATD = 0x7E;	
	dataInst();
	busyFlag();
}
void clearPreviousFlesh(void)		
{
	LATD = position;
	commandInst();
	busyFlag();
	LATD = 0x20;
	dataInst();
	busyFlag();
}
void rotateLed(void)
{
	unsigned char counter = 0;
	PORTE = 0x00;
	while(counter<10)
	{	
		PORTE = 0x01;
		delay250ms();
		while(PORTEbits.RE2 != 1)
		{
			PORTE <<= 1;
			delay250ms();
		}
		++counter;
	}
	PORTE = 0x00;
}
void blinkLed(void)
{
	unsigned char counter = 0;
	PORTE = 0x00;
	while(counter<10)
	{
		PORTEbits.RE0 = ~PORTEbits.RE0;
		delay250ms();
		++counter;
	}
	PORTE = 0x00;
}
void shiftLed(void)
{
	unsigned char counter = 0;
	PORTE = 0x01;
	delay250ms();
	while(counter<10)
	{			
		while(PORTEbits.RE2 != 1)
		{
			PORTE <<= 1;
			delay250ms();
		}
		while(PORTEbits.RE0 != 1)
		{
			PORTE >>= 1;
			delay250ms();
		}
		++counter;	
	}	
	PORTE = 0x00;
}
void swapLed(void)
{
	unsigned char counter = 0;
	while(counter<10)
	{
		PORTE = 0x05;
		delay250ms();
		PORTE = 0x02;
		delay250ms();
		++counter;
	}
	PORTE = 0x00;
}