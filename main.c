/*
 * main.c
 */

#include <msp430g2553.h>
#include <float.h>
#include "delay.h"
#include "i2c.h"

volatile unsigned long int KP_RAW;
volatile unsigned long int KI_RAW;
volatile unsigned long int KD_RAW;

#define gyro_sensitivity 65.5
#define PI 3.14159265
#define RAD_TO_DEG 57.29578
#define ratio 0.98

// 12.5, 0.95, 9.0

int i;
int accel_x;
int accel_y;
int accel_z;
int gy_x;
int gy_y;
int gy_z;
float accel_x_angle;
float gyro_y_rate;
float gyro_y_angle;
float final_angle;
float pterm;
float iterm;
float dterm;
float total;
float last_angle;
float KI;
float KP;
float KD;

void get_accel_angle()
{

	accel_x = i2c_read_byte(0x68,0x3B) << 8;
	accel_x = accel_x | i2c_read_byte(0x68, 0x3C);

	accel_y = i2c_read_byte(0x68,0x3D) << 8;
	accel_y = accel_y | i2c_read_byte(0x68, 0x3E);

	accel_z = i2c_read_byte(0x68,0x3F) << 8;
	accel_z = accel_z | i2c_read_byte(0x68, 0x40);

	// small angle approximation
	accel_x_angle = accel_x/182.0 - 5.5;
}

void get_gyro_rate()
{
	gy_x = i2c_read_byte(0x68,0x43) << 8;
	gy_x = gy_x | i2c_read_byte(0x68, 0x44);

	gy_y = i2c_read_byte(0x68,0x45) << 8;
	gy_y = gy_y | i2c_read_byte(0x68, 0x46);

	gy_z = i2c_read_byte(0x68,0x47) << 8;
	gy_z = gy_z | i2c_read_byte(0x68, 0x48);

	gyro_y_rate = (gy_y/gyro_sensitivity * -1.0) - 3.0;
}

void get_kp_raw()
{
	// set ADC
	ADC10CTL1 = INCH_0 + ADC10DIV_3;
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + MSC + ADC10ON;
	ADC10AE0 = BIT0 + BIT1 + BIT2;

	delay_usec(10);

	for(i = 0; i < 100; ++i) {
		ADC10CTL0 |= ENC + ADC10SC;                            // Sampling and conversion start
		while ( ADC10CTL1 & ADC10BUSY );                       // Wait for ADC to complete
		KP_RAW += 1023 - ADC10MEM;
	}

	KP_RAW /= 100;

	ADC10CTL0 &= ~ENC;
}

void get_ki_raw()
{
	// set ADC
	ADC10CTL1 = INCH_1 + ADC10DIV_3;
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + MSC + ADC10ON;
	ADC10AE0 = BIT0 + BIT1 + BIT2;

	delay_usec(10);

	for(i = 0; i < 100; ++i) {
		ADC10CTL0 |= ENC + ADC10SC;                            // Sampling and conversion start
		while ( ADC10CTL1 & ADC10BUSY );                       // Wait for ADC to complete
		KI_RAW += 1023 - ADC10MEM;
	}

	KI_RAW /= 100;

	ADC10CTL0 &= ~ENC;
}

void get_kd_raw()
{
	// set ADC
	ADC10CTL1 = INCH_2 + ADC10DIV_3;
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + MSC + ADC10ON;
	ADC10AE0 = BIT0 + BIT1 + BIT2;


	delay_usec(10);

	for(i = 0; i < 100; ++i) {
		ADC10CTL0 |= ENC + ADC10SC;                            // Sampling and conversion start
		while ( ADC10CTL1 & ADC10BUSY );                       // Wait for ADC to complete
		KD_RAW += 1023 - ADC10MEM;
	}

	KD_RAW /= 100;

	ADC10CTL0 &= ~ENC;
}


void main(void)
{
	// delay
	delay_usec(500);

	// stop watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	// set clock speeds
	BCSCTL1 = CALBC1_16MHZ;
	DCOCTL = CALDCO_16MHZ;

	// turn on P1.3 through P1.5 for output
	P1DIR |= BIT3 + BIT4 + BIT5;

	// turn on P1.7 for input
	P1DIR &= ~BIT7;

	// motor controls
	P2DIR |= BIT4;
	P2DIR |= BIT5;

	//test frequency
	P2DIR |= BIT2;

	// PWM
	P1DIR |= BIT6;
	P1SEL |= BIT6;
	CCR0 = 16000-1;             // PWM Period
	CCTL1 = OUTMOD_7;          // CCR1 reset/set
	CCR1 = 2500;                // CCR1 PWM duty cycle
	TACTL = TASSEL_2 | MC_1;   // SMCLK, up mode

	// initialize MPU6050
	// sample rate at 1000Hz
	i2c_write_byte(0x68, 0x19, 0x07);

	// disable gyro self tests, scale 500 degrees/sec
	i2c_write_byte(0x68, 0x1B, 0x08);

	//Sets clock source to gyro reference w/ PLL
	i2c_write_byte(0x68, 0x6B, 0x02);

	P2OUT &= ~BIT5;
	P2OUT |= BIT4;

	i = 0;
	accel_x_angle = 0.0;
	gyro_y_rate = 0.0;
	gyro_y_angle = 0.0;
	final_angle = 0.0;
	pterm = 0.0;
	iterm = 0.0;
	dterm = 0.0;
	last_angle = 0.0;
	total = 0.0;
	KP = 7.0;
	KI = 1.19;
	KD = 17.1;

	for(;;) {
		if (P1IN & BIT7) {
			P2OUT ^= BIT2;

			// measure accel angle
			get_accel_angle();

			// measure gyro rate
			get_gyro_rate();

			// calculate gyro angle
			gyro_y_angle += (float)(gyro_y_rate * 0.0162);

			// complimentary filter for final angle
			final_angle = ratio * (final_angle + gyro_y_rate*0.0162) + (1 - ratio) * accel_x_angle;

			// math for calculating error in PID
			pterm = KP * final_angle;
			iterm += KI * final_angle;
			dterm = KD * (final_angle - last_angle);
			last_angle = final_angle;
			total = pterm + iterm + dterm;

			if(total > 0) {
				CCR1 = (int)(total * 16000 / 90);
				P2OUT &= ~BIT5;
				P2OUT |= BIT4;
			}

			else {
				CCR1 = (int)(total * -16000 / 90);
				P2OUT &= ~BIT4;
				P2OUT |= BIT5;
			}
		}

		else {
				CCR1 = 0;
				accel_x_angle = 0.0;
				gyro_y_rate = 0.0;
				gyro_y_angle = 0.0;
				final_angle = 0.0;
				pterm = 0.0;
				iterm = 0.0;
				dterm = 0.0;
				last_angle = 0.0;
				total = 0.0;
				KP_RAW = 0;
				KI_RAW = 0;
				KD_RAW = 0;

				get_kp_raw();
				KP = (float)KP_RAW / 51.15;
				get_ki_raw();
				KI = (float)KI_RAW / 341.0;
				get_kd_raw();
				KD = (float)KD_RAW / 11.15;
		}
	}
}
