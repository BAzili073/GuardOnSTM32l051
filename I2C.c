#include "I2C.h"

#define I2C_PORT I2C1
#define I2C_SCL_PIN GPIO_PIN_8
#define I2C_SCL_PORT GPIOB
#define I2C_SDA_PIN GPIO_PIN_9
#define I2C_SDA_PORT GPIOB

void I2C_Init(){
	GPIO_InitTypeDef initSrtuct;

	initSrtuct.Alternate = GPIO_AF4_I2C1;
	initSrtuct.Mode = GPIO_MODE_AF_OD;
	initSrtuct.Pull = GPIO_PULLUP;
	initSrtuct.Pin = I2C_SCL_PIN;
	initSrtuct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(I2C_SCL_PORT, &initSrtuct);

	initSrtuct.Alternate = GPIO_AF4_I2C1;
	initSrtuct.Mode = GPIO_MODE_AF_OD;
	initSrtuct.Pull = GPIO_PULLUP;
	initSrtuct.Pin = I2C_SDA_PIN;
	initSrtuct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(I2C_SDA_PORT, &initSrtuct);



	__I2C1_CLK_ENABLE();
	I2C_HandleTypeDef initS;
	initS.Instance = I2C_PORT;
	initS.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	initS.Init.ClockSpeed = 10000;
	initS.Init.DualAddressMode = 0;
	initS.Init.GeneralCallMode = 0;
	initS.Init.NoStretchMode = 0;
	HAL_I2C_Init(&initS);


//	/* (1) Timing register value is computed with the AN4235 xls file,
//	fast Mode @400kHz with I2CCLK = 16MHz, rise time = 100ns,
//	fall time = 10ns */
//	/* (2) Periph enable, address match interrupt enable */
//	/* (3) 7-bit address = 0x5A */
//	/* (4) Enable own address 1 */
//	I2C1->TIMINGR = (uint32_t)0x00300619; /* (1) */
//	I2C1->CR1 = I2C_CR1_PE | I2C_CR1_ADDRIE; /* (2) */
//	I2C1->OAR1 |= (uint32_t)(I2C1_OWN_ADDRESS << 1); /* (3) */
//	I2C1->OAR1 |= I2C_OAR1_OA1EN; /* (4) */


//	I2C_PORT->CR2 |= I2C_CR2_ITEVTEN;
//	I2C_PORT->CR2 |= I2C_CR2_ITBUFEN;
//	I2C_PORT->CR2 |= I2C_CR2_FREQ_1;

//	NVIC_EnableIRQ(I2C1_EV_IRQn);
//	NVIC_EnableIRQ(I2C1_ER_IRQn);

	I2C_PORT->CR1 |= I2C_CR1_PE;
	I2C_PORT->CR1 |= I2C_CR1_ACK;
}


void I2C_SLAVE_TRACSMITTED(){
	uint32_t I2C_InterruptStatus = I2C1->ISR; /* Get interrupt status */
	/* Check address match */
	if((I2C_InterruptStatus & I2C_ISR_ADDR) == I2C_ISR_ADDR)
	{
	I2C1->ICR |= I2C_ICR_ADDRCF; /* Clear address match flag */
	/* Check if transfer direction is read (slave transmitter) */
	if((I2C1->ISR & I2C_ISR_DIR) == I2C_ISR_DIR)
	{
	I2C1->CR1 |= I2C_CR1_TXIE; /* Set transmit IT */
	}
	}
	else if((I2C_InterruptStatus & I2C_ISR_TXIS) == I2C_ISR_TXIS)
	{
	I2C1->CR1 &=~ I2C_CR1_TXIE; /* Disable transmit IT */
	I2C1->TXDR = I2C_BYTE_TO_SEND; /* Byte to send */
	}
}


void I2C_SLAVE_RECIVE(){
	uint32_t I2C_InterruptStatus = I2C1->ISR; /* Get interrupt status */
	if((I2C_InterruptStatus & I2C_ISR_ADDR) == I2C_ISR_ADDR)
	{
	I2C1->ICR |= I2C_ICR_ADDRCF; /* Address match event */
	}
	else if((I2C_InterruptStatus & I2C_ISR_RXNE) == I2C_ISR_RXNE)
	{
	/* Read receive register, will clear RXNE flag */
	if(I2C1->RXDR == I2C_BYTE_TO_SEND)
	{
	/* Process */
	}
	}
}


void i2c_write_byte(char address, char value){
	I2C_PORT->CR1 |= I2C_CR1_START;
	while((I2C_PORT->SR1 & I2C_SR1_SB) == 0);
	int x = I2C_PORT->SR1;
	I2C_PORT->DR = address;
	while((I2C_PORT->SR1 &  I2C_SR1_ADDR) == 0){
			if ((I2C_PORT->SR1 &  I2C_SR1_AF) == I2C_SR1_AF){
				I2C_PORT->CR1 |= I2C_CR1_STOP;
				return;
			}
	}
	x = I2C_PORT->SR1;
	x = I2C_PORT->SR2;
	I2C_PORT->DR = value;
	while((I2C_PORT->SR1 &  I2C_SR1_TXE) == 0);
	I2C_PORT->CR1 |= I2C_CR1_STOP;
}

int i2c_read_2byte(char address){
	I2C_PORT->CR1 |= I2C_CR1_START;
	while((I2C_PORT->SR1 &  I2C_SR1_SB) == 0);
	int x = I2C_PORT->SR1;
	I2C_PORT->DR = address;
	while((I2C_PORT->SR1 &  I2C_SR1_ADDR) == 0)
	{
		if ((I2C_PORT->SR1 &  I2C_SR1_AF) == I2C_SR1_AF){
			I2C_PORT->CR1 |= I2C_CR1_STOP;
			return 0;
		}
	}
	I2C_PORT->SR1;
	I2C_PORT->SR2;
	while((I2C_PORT->SR1 &  I2C_SR1_RXNE) == 0);
	int tmp = I2C_PORT->DR;
	tmp = tmp<<8;

	while((I2C_PORT->SR1 &  I2C_SR1_RXNE) == 0);
	tmp |= I2C_PORT->DR;
	I2C_PORT->CR1 |= I2C_CR1_STOP;
	return tmp;
}


void i2c_write_reg(char reg, char value){
	I2C_PORT->CR1 |= I2C_CR1_START;
	while((I2C_PORT->SR1 &  I2C_SR1_SB) == 0);
	int x = I2C_PORT->SR1;
//	I2C_PORT->DR = I2C_ADRESS_WRITE;
	while((I2C_PORT->SR1 &  I2C_SR1_ADDR) == 0)
		{
			if ((I2C_PORT->SR1 &  I2C_SR1_AF) == I2C_SR1_AF){
				return 0;
			}
	}
	x = I2C_PORT->SR1;
	x = I2C_PORT->SR2;
	I2C_PORT->DR = reg;
	while((I2C_PORT->SR1 &  I2C_SR1_TXE) == 0);
	I2C_PORT->DR = value;
	while((I2C_PORT->SR1 &  I2C_SR1_TXE) == 0);
	I2C_PORT->CR1 |= I2C_CR1_STOP;
}


char read_reg(char reg){
	I2C_PORT->CR1 |= I2C_CR1_START;
	int x;

	while((I2C_PORT->SR1 &  I2C_SR1_SB) == 0);
	x = I2C_PORT->SR1;
//	I2C_PORT->DR = I2C_ADRESS_WRITE;


	while((I2C_PORT->SR1 &  I2C_SR1_ADDR) == 0)
	{
		if ((I2C_PORT->SR1 &  I2C_SR1_AF) == I2C_SR1_AF){
			return 0;
		}
	}

	I2C_PORT->SR1;
	I2C_PORT->SR2;

	I2C_PORT->DR = reg;
	while((I2C_PORT->SR1 &  I2C_SR1_TXE) == 0);

	I2C_PORT->CR1 |= I2C_CR1_START;
	while((I2C_PORT->SR1 &  I2C_SR1_SB) == 0);
	x = I2C_PORT->SR1;
//	I2C_PORT->DR = I2C_ADRESS_READ;

/*	while((I2C_PORT->SR1 &  I2C_SR1_AF) == I2C_SR1_AF){
		I2C_PORT->DR = I2C_ADRESS_READ;
		delay();
	}
*/
	while((I2C_PORT->SR1 &  I2C_SR1_ADDR) == 0)
		{
			if ((I2C_PORT->SR1 &  I2C_SR1_AF) == I2C_SR1_AF){
				return 0;
			}
		}
	I2C_PORT->SR1;
	I2C_PORT->SR2;
	while((I2C_PORT->SR1 &  I2C_SR1_RXNE) == 0);
	char tmp = I2C_PORT->DR;
	I2C_PORT->CR1 |= I2C_CR1_STOP;
	return tmp;
}
