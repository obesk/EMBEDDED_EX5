/*
 * File:   timer.c
 * Author: obe
 *
 * Created on March 3, 2025, 11:03 AM
 */

#include "timer.h"
#include <xc.h>
#define MAX_DELAY 200

#define FCY 72000000
#define MAX_UINT16 65535

// TODO: here I implemented an adaptable prescaler, we should ask prof Simetti
// if this is a good approach

void tmr_setup_period(int timer, int ms) {
	if (ms <= 0) {
		return;
	}

	const int prescalers[] = { 1, 8, 64, 256 };

	const float s = (float)ms / 1000.;

	// finding the smallest (best) prescaler who can wait the passed ms
	float p = s * ((float)FCY / (float)MAX_UINT16);
	int i = 0;
	while (p > prescalers[i]) {
		++i;
	}
	// TODO: here we could return an error value if the ms passed exceed the
	// value of the prescaler
	const int p_type = i;

	const int clocks = (int)(s * (float)FCY / (float)(prescalers[p_type]));

	switch (timer) {
	case TIMER1:
		T1CONbits.TON = 0;		  // stopping the timer
		TMR1 = 0;				  // resetting the counter
		T1CONbits.TCKPS = p_type; // setting the prescaler
		PR1 = clocks;	   // loads the maximum number the timer can reach
		IFS0bits.T1IF = 0; // setting the flag to 0
		T1CONbits.TON = 1; // re-starting the timer
		break;

	case TIMER2: // same as the other timer
		T2CONbits.TON = 0;
		TMR2 = 0;
		T2CONbits.TCKPS = p_type;
		PR2 = clocks;
		IFS0bits.T2IF = 0;
		T2CONbits.TON = 1;
		break;

	default:
		break;
	}
}

int tmr_wait_period(int timer) {

	int ret = 0;
	switch (timer) {
	case TIMER1:
		if (IFS0bits.T1IF == 1) {
			ret = 1;
		} else {
			while (IFS0bits.T1IF == 0) {
				;
			}
		}
		IFS0bits.T1IF = 0;
		break;
	case TIMER2:
		if (IFS0bits.T2IF == 1) {
			ret = 1;
		} else {
			while (IFS0bits.T2IF == 0) {
				;
			}
		}
		IFS0bits.T2IF = 0;
		break;
	}

	return ret;
}

void tmr_wait_ms(int timer, int ms) {
	// with the adaptable prescaler I'm sure that we can wait 1ms exactly
	tmr_setup_period(timer, 1);

	for (int i = 0; i < ms; ++i) {
		tmr_wait_period(timer);
	}
}
