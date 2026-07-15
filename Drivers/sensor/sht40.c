#include <stdio.h>
#include "tremo_delay.h"
#include "log.h"
#include "I2C_A.h"
#include "bsp.h"
#include "sht40.h"

static float gxht40_temp_record=10,gxht40_hum_record=40;

//-----------------------------------------------------------------------------
uint8_t SHT40_CheckSum_CRC8(uint8_t* Result, uint8_t offset)
{
	uint8_t data[2];
	data[0] = Result[offset];
	data[1] = Result[offset+1];

	uint32_t POLYNOMIAL = 0x131;
	uint8_t crc = 0xFF;
	uint8_t bit = 0;
	uint8_t byteCtr = 0;

	//calculates 8-Bit checksum with given polynomial
	for (byteCtr = 0; byteCtr < 2; ++byteCtr)
	{
		crc ^= (data[byteCtr]);
		for (bit = 8; bit > 0; --bit)
		{
			if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
			else crc = (crc << 1);
		}
	}
	if (crc == Result[offset+2]) {
		return 1;
	}
	else {
		return 0;
	}
}

//-----------------------------------------------------------------------------
uint8_t check_sht40_connect(void)
{
	uint8_t rxdata[6];
	uint8_t data[1] = {SHT40_CMD_READ_SERIAL};
	I2C_GPIO_MODE_Config();
	I2C_Write_Len(SHT40_I2C_ADDR,0x01,1,data);
	delay_ms(10);
	I2C_Read_Len(SHT40_I2C_ADDR,0x01,6,rxdata);
	if((SHT40_CheckSum_CRC8(rxdata,0)==1)&&(SHT40_CheckSum_CRC8(rxdata,3)==1))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//-----------------------------------------------------------------------------
void SHT40_Read(sht3x_data_t *sht40_data, sht40_precision_t precision)
{
	uint8_t rxdata[6];
	uint8_t txdata[1];
	uint8_t meas_delay;
	uint16_t raw_t,raw_rh;
	uint8_t check_number=0;
	bool read_status=0;

	// select measurement command and duration according to the precision
	switch(precision)
	{
		case SHT40_PRECISION_MEDIUM:
			txdata[0]=SHT40_CMD_MEAS_MEDIUM;
			meas_delay=5;//~4.5 ms
			break;
		case SHT40_PRECISION_LOW:
			txdata[0]=SHT40_CMD_MEAS_LOW;
			meas_delay=2;//~1.6 ms
			break;
		case SHT40_PRECISION_HIGH:
		default:
			txdata[0]=SHT40_CMD_MEAS_HIGH;
			meas_delay=10;//~8.2 ms
			break;
	}

	I2C_GPIO_MODE_Config();

	do
	{
		check_number++;
		read_status=0;
		if(I2C_Write_Len(SHT40_I2C_ADDR,0x01,1,txdata)==0)   // 0 = ACK (success), 1 = NACK
		{
			delay_ms(meas_delay);//measurement duration (depends on precision)
			if(I2C_Read_Len(SHT40_I2C_ADDR,0x01,6,rxdata)==0)   // 0 = ACK (success), 1 = NACK
			{
				if((SHT40_CheckSum_CRC8(rxdata,0)==1)&&(SHT40_CheckSum_CRC8(rxdata,3)==1))
				{
					read_status=1;
				}
			}
		}
		if(read_status==0)
		{
			delay_ms(20);
		}
	}while(read_status==0&&check_number<4);

	if(read_status==1)
	{
		raw_t =(rxdata[0]<<8)+rxdata[1];
		raw_rh=(rxdata[3]<<8)+rxdata[4];

		sht40_data->temp_sht=175.0*raw_t/65535.0-45.0;
		sht40_data->hum_sht =125.0*raw_rh/65535.0-6.0;

		if(sht40_data->temp_sht>125)
		{
			sht40_data->temp_sht=gxht40_temp_record;
		}
		else if(sht40_data->temp_sht<-40)
		{
			sht40_data->temp_sht=gxht40_temp_record;
		}

		if(sht40_data->hum_sht>100)
		{
			sht40_data->hum_sht=100;
		}
		else if(sht40_data->hum_sht<0)
		{
			sht40_data->hum_sht=0;
		}

		gxht40_temp_record=sht40_data->temp_sht;
		gxht40_hum_record=sht40_data->hum_sht;
	}
	else
	{
		sht40_data->temp_sht=3276.7;
		sht40_data->hum_sht=6553.5;
	}
}
/******************* (C) COPYRIGHT Dragino *****END OF FILE****/
