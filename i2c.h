/*
 * i2c.h
 *
 *  Created on: May 13, 2013
 *      Author: Andrew
 */

#ifndef I2C_H_
#define I2C_H_

void i2c_delay(void);
void i2c_start(void);
void i2c_stop(void);
char i2c_rx(char ack);
void i2c_tx(char d);
char i2c_read_byte(char addr, char byte);
void i2c_write_byte(char i2c_addr, char reg_addr, char val);

#endif /* I2C_H_ */
