/*	Partner(s) Name & E-mail: Paul Schneider [pschn002@ucr.edu], Jonathan Le [jle041@ucr.edu]
 *	Lab Section: 023
 *	Assignment: Lab #1 Exercise #1
 *	Exercise Description: AVR and Programming Review, three LED series blinking, toggle LED blinking
 *	
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <scheduler.h>

// State machine declarations and configuration
enum SMState_Sequence {
	SEQUENCE_INIT, SEQUENCE_LED0, SEQUENCE_LED1, SEQUENCE_LED2, SEQUENCE_DEFAULT
};
enum SMState_Blink {
	BLINK_INIT, BLINK_ON, BLINK_OFF, BLINK_DEFAULT
};
enum SMState_Control {
	CONTROL_INIT, CONTROL_IDLE, CONTROL_PRESSED, CONTROL_PAUSED, CONTROL_DEFAULT
};
enum SMState_Display {
	DISPLAY_INIT, DISPLAY_OUT, DISPLAY_DEFAULT
};

unsigned long tasksPeriod_Blink = 500;
unsigned long tasksPeriod_Sequence = 1000;
unsigned long tasksPeriod_Control = 50;
unsigned long tasksPeriod_Display = 250;
// TODO Add GCD calculation
// tmp_gcd = ...
unsigned long tasksPeriodGCD = 50; // Start count from here, down to 0. Default 1ms

unsigned char tasksNum = 4; // Number of tasks in the scheduler. Default 0 tasks
task tasks[4];


// System global control variables
unsigned char sysPause = 0;
unsigned char buttonProcessed = 1;
unsigned char playerScore = 0;

unsigned char PB_blink_out = 0x00;
unsigned char PB_sequence_out = 0x00;



int SMTick_Sequence(int state) {
	
	switch (state) {
		case SEQUENCE_INIT:
			state = sysPause ? state : SEQUENCE_LED0;
			break;
		
		case SEQUENCE_LED0:
			state = sysPause ? state : SEQUENCE_LED1;
			break;
		
		case SEQUENCE_LED1:
			state = sysPause ? state : SEQUENCE_LED2;
			break;
		
		case SEQUENCE_LED2:
			state = sysPause ? state : SEQUENCE_LED0;
			break;
		
		default:
			state = SEQUENCE_DEFAULT;
			break;
	}
	
	switch (state) {
		case SEQUENCE_LED0:
			PB_sequence_out = 0x01;
			break;
		
		case SEQUENCE_LED1:
			PB_sequence_out = 0x02;
			break;

		case SEQUENCE_LED2:
			PB_sequence_out = 0x04;
			break;
		
		default:
			PB_sequence_out = 0x0F;
			break;
	}
	
	return state;
}

int SMTick_Blink(int state) {
	
	switch (state) {
		case BLINK_INIT:
			state = sysPause ? state : BLINK_ON;
			break;
			
		case BLINK_ON:
			state = sysPause ? state : BLINK_OFF;
			break;
			
		case BLINK_OFF:
			state = sysPause ? state : BLINK_ON;
			break;
		
		default:
			state = BLINK_DEFAULT;
	}
	
	switch (state) {
		case BLINK_ON:
			PB_blink_out = 0x08;
			break;
			
		case BLINK_OFF:
			PB_blink_out = 0x00;
			break;
			
		default:
			PB_blink_out = 0x0F;
			break;
	}
	
	return state;
}



int SMTick_Control(int state) {
	
	switch (state) {
		case CONTROL_INIT:
			state = CONTROL_IDLE;
			break;
		
		case CONTROL_IDLE:
			if (PINA & 0x01) {
				 buttonProcessed = 0;
				 state = CONTROL_PRESSED;
			}
			else state = CONTROL_IDLE;
			break;
		
		case CONTROL_PRESSED:
			state = (PINA & 0x01) ? CONTROL_PRESSED : CONTROL_IDLE;
			break;
		
		default:
			state = CONTROL_DEFAULT;
			break;
	}
	
	switch (state) {
		case CONTROL_INIT:
			
			break;
		
		case CONTROL_IDLE:
			
			break;
		
		case CONTROL_PRESSED:
			if (!buttonProcessed) {
				// Handle logic of button press
				if (sysPause) {
					// Unpause system if already paused
					sysPause = 0;
				}
				else {
					// Check scoring conditions on new pause/press
					if (tasks[0].state == SEQUENCE_LED1) {
						// Hit success, increment score
						playerScore++;
					}
					else {
						// Miss, decrement down to zero
						playerScore = playerScore ? playerScore - 1 : 0;
					}
				}
			}
			break;

		default:
			playerScore = 69;
			break;
	}
	
	return state;
}

int SMTick_Display(int state) {
	static unsigned char PB_out;
	static unsigned char PB_out_last;	

	switch (state) {
		case DISPLAY_INIT:
			state = DISPLAY_OUT;
			break;
		
		case DISPLAY_OUT:
			state = DISPLAY_OUT;
			break;

		default:
			state = DISPLAY_DEFAULT;
			break;
	}
	
	switch (state) {
		case DISPLAY_OUT:
			PB_out = PB_blink_out | PB_sequence_out;
			//if (PB_out != PB_out_last) {
				PORTB = PB_out;
			//}
			// TODO LCD output
			break;
		
		default:
			PORTB = 0xFF;
			break;
	}
	
	return state;
}





int main(void)
{
	DDRB = 0xFF; // Set port B to output
	PORTB = 0xFF; // Init port B to 0s
	DDRA = 0x00;
	PORTA = 0xFF;

	TimerSet(500); //TODO GCD
	TimerOn();

	tasks[0].TickFct = &SMTick_Sequence;
	tasks[0].state = 0;
	tasks[0].period = 10;
	tasks[0].elapsedTime = 0;

	tasks[1].TickFct = &SMTick_Blink;
	tasks[1].state = 0;
	tasks[1].period = 20;
	tasks[1].elapsedTime = 0;

	tasks[2].TickFct = &SMTick_Control;
	tasks[2].state = 0;
	tasks[2].period = 1;
	tasks[2].elapsedTime = 0;

	tasks[3].TickFct = &SMTick_Display;
	tasks[3].state = 0;
	tasks[3].period = 5;
	tasks[3].elapsedTime = 0;

	while(1) {
		
	}
}



