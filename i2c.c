#include <msp430g2553.h>
#include "i2c.h"
#include "delay.h"

void i2c_delay(void) {
	// for 200KHz period
	delay_usec(10);
}

void i2c_start(void) {
	// set SDA for output
	P2DIR |= BIT1;

	// set SDA to output 1
	P2DIR |= BIT1;

	// delay
	i2c_delay();

	// set SCL for output
	P2DIR |= BIT0;

	// set SCL to output 1
	P2OUT |= BIT0;

	// delay
	i2c_delay();

	// output 0 on SDA
	P2OUT &= ~BIT1;

	// delay
	i2c_delay();

	// output 0 on SCL
	P2OUT &= ~BIT0;

	// delay
	i2c_delay();
}

void i2c_stop(void) {
	// set SDA for output
	P2DIR |= BIT1;

	// set SDA to output 0
	P2OUT &= ~BIT1;

	// delay
	i2c_delay();

	// set SCL for output
	P2DIR |= BIT0;

	// set SCL to output 1
	P2OUT |= BIT0;

	// delay
	i2c_delay();

	// output 1 on SDA
	P2OUT |= BIT1;

	// delay
	i2c_delay();
}

char i2c_rx(char ack) {
	char i, d = 0;

	// set SDA for output and output 1
	P2DIR |= BIT1;
	P2OUT |= BIT1;

	i2c_delay();

	// return SDA for input
	P2DIR &= ~BIT1;

	for (i = 0; i < 8; ++i) {
		d <<= 1;

		P2DIR |= BIT0;
		P2OUT |= BIT0;

		// clock stretching
		P2DIR &= ~BIT0;
		while ((P2IN & BIT0) == 0);

		// set SCL for output and write high
		P2DIR |= BIT0;
		P2OUT |= BIT0;
		i2c_delay();

		if(P2IN & BIT1)
			d |= 1;

		// write low on SCL
		P2OUT &= ~BIT0;
		i2c_delay();
	}

	// set SDA for output
	P2DIR |= BIT1;

	ack ? (P2OUT &= ~BIT1) : (P2OUT |= BIT1);

	// set SCL for output and output 1
	P2DIR |= BIT0;
	P2OUT |= BIT0;

	i2c_delay();

	P2OUT &= ~BIT0;
	P2OUT |= BIT1;

	return d;
}

void i2c_tx(char d) {
	char x;

	// set SCL and SDA for outputs
	P2DIR |= BIT0;
	P2DIR |= BIT1;

	i2c_delay();

	for(x = 8; x; --x) {
		if(d & 0x80)
			P2OUT |= BIT1;
		else
			P2OUT &= ~BIT1;
		P2OUT |= BIT0;
		d <<= 1;
		i2c_delay();
		P2OUT &= ~BIT0;
		i2c_delay();
	}

	P2OUT |= BIT1;
	P2OUT |= BIT0;

	i2c_delay();

	P2OUT &= ~BIT0;

	// i2c_delay();
}

char i2c_read_byte(char addr, char byte) {
		char data;
		i2c_start();
		i2c_tx(addr << 1);
		i2c_tx(byte);
		i2c_start();
		i2c_tx((addr << 1) | 1);
		data = i2c_rx(0);
		i2c_stop();
		return data;
}

void i2c_write_byte(char i2c_addr, char reg_addr, char val) {
	i2c_start();
	i2c_tx(i2c_addr << 1);
	i2c_tx(reg_addr);
	i2c_tx(val);
	i2c_stop();
}

