
#ifndef DELAY_H
#define DELAY_H

	#define delay		delay_ms
	#define _delay_us	delay_us
	#define _delay_ms	delay_ms
	extern void delay_us(volatile unsigned long d);
	extern void delay_ms(volatile unsigned long d);
	
#endif //DELAY_H
