/*
 * Copyright (c) 2014-2021 Rtrobot. <admin@rtrobot.org>
 *  <http://rtrobot.org>
 ***********************************************************************
 */
#ifndef AS7341_H
#define AS7341_H
// http://rtrobot.org

#include <Arduino.h>

#define AS7341_I2CADDR 0x39

#define AS7341_REG_ASTATUS 0x60
#define AS7341_REG_ITIME_L 0x63
#define AS7341_REG_ITIME_M 0x64
#define AS7341_REG_ITIME_H 0x65
#define AS7341_REG_CONFIG 0x70
#define AS7341_REG_STAT 0x71
#define AS7341_REG_EDGE 0x72
#define AS7341_REG_GPIO 0x73
#define AS7341_REG_LED 0x74
#define AS7341_REG_ENABLE 0x80
#define AS7341_REG_ATIME 0x81
#define AS7341_REG_WTIME 0x83
#define AS7341_REG_SP_TH_L_LSB 0x84
#define AS7341_REG_SP_TH_L_MSB 0x85
#define AS7341_REG_SP_TH_H_LSB 0x86
#define AS7341_REG_SP_TH_H_MSB 0x87
#define AS7341_REG_AUXID 0x90
#define AS7341_REG_REVID 0x91
#define AS7341_REG_DEVICE 0x92
#define AS7341_REG_STATUS 0x93
#define AS7341_REG_ASTATUS2 0x94

// #define AS7341_REG_CH0_DATA_L 0X61
// #define AS7341_REG_CH0_DATA_H 0X62
// #define AS7341_REG_CH1_DATA_L 0X66
// #define AS7341_REG_CH1_DATA_H 0X67
// #define AS7341_REG_CH2_DATA_L 0X68
// #define AS7341_REG_CH2_DATA_H 0X69
// #define AS7341_REG_CH3_DATA_L 0X6A
// #define AS7341_REG_CH3_DATA_H 0X6B
// #define AS7341_REG_CH4_DATA_L 0X6C
// #define AS7341_REG_CH4_DATA_H 0X6D
// #define AS7341_REG_CH5_DATA_L 0X6E
// #define AS7341_REG_CH5_DATA_H 0X6F
#define AS7341_REG_CH0_DATA_L 0X95
#define AS7341_REG_CH0_DATA_H 0X96
#define AS7341_REG_CH1_DATA_L 0X97
#define AS7341_REG_CH1_DATA_H 0X98
#define AS7341_REG_CH2_DATA_L 0X99
#define AS7341_REG_CH2_DATA_H 0X9A
#define AS7341_REG_CH3_DATA_L 0X9B
#define AS7341_REG_CH3_DATA_H 0X9C
#define AS7341_REG_CH4_DATA_L 0X9D
#define AS7341_REG_CH4_DATA_H 0X9E
#define AS7341_REG_CH5_DATA_L 0X9F
#define AS7341_REG_CH5_DATA_H 0XA0

#define AS7341_REG_STATUS2 0xA3
#define AS7341_REG_STATUS3 0xA4
#define AS7341_REG_STATUS5 0xA6
#define AS7341_REG_STATUS6 0xA7
#define AS7341_REG_CFG0 0xA9
#define AS7341_REG_CFG1 0xAA
#define AS7341_REG_CFG3 0xAC
#define AS7341_REG_CFG6 0xAF
#define AS7341_REG_CFG8 0xB1
#define AS7341_REG_CFG9 0xB2
#define AS7341_REG_CFG10 0xB3
#define AS7341_REG_CFG12 0xB5
#define AS7341_REG_PERS 0xBD
#define AS7341_REG_GPIO2 0xBE
#define AS7341_REG_ASTEP_L 0xCA
#define AS7341_REG_ASTEP_H 0xCB
#define AS7341_REG_AGC_GAIN_MAX 0xCF
#define AS7341_REG_AZ_CONFIG 0xD6
#define AS7341_REG_FD_TIME1 0xD8
#define AS7341_REG_FD_TIME2 0xDA
#define AS7341_REG_FD_CFG0 0xD7
#define AS7341_REG_FD_STATUS 0xDB
#define AS7341_REG_INTENAB 0xF9
#define AS7341_REG_CONTROL 0xFA
#define AS7341_REG_FIFO_MAP 0xFC
#define AS7341_REG_LVL 0XFD
#define AS7341_REG_FE_L 0XFE
#define AS7341_REG_FE_H 0XFF

#define AS7341_GAIN_0_5 0x00
#define AS7341_GAIN_1 0x01
#define AS7341_GAIN_2 0x02
#define AS7341_GAIN_4 0x03
#define AS7341_GAIN_8 0x04
#define AS7341_GAIN_16 0x05
#define AS7341_GAIN_32 0x06
#define AS7341_GAIN_64 0x07
#define AS7341_GAIN_128 0x08
#define AS7341_GAIN_256 0x09
#define AS7341_GAIN_512 0x0A

#define AS7341_SMUX_INIT 0x00
#define AS7341_SMUX_READ 0x08
#define AS7341_SMUX_WRITE 0x10

class AS7341
{
public:
	AS7341();
	boolean begin(void);
	bool Get_Id(void);
	void SetEnable(bool status);
	void SpectralMeasurement(bool status);
	void ControlLed(bool status, uint8_t value);
	bool WaitDataReady(void);
	void FDConfig(void);
	void ReadAllChannels(uint16_t *readings_buffer);
	uint8_t ReadFlickerData(void);
	void F1F4_Clear_NIR(void);
	void F5F8_Clear_NIR(void);
	void SetEnableSMUX(void);
	void SetSMUX(uint8_t value);
	void SetBank(bool status);
	void EnableFlickerDetection(bool status);
	void SetIntegrationTime(uint8_t atime, uint16_t astep);
	void SetGain(uint8_t value);
	void SetLowPower(bool status);

private:
	uint8_t _i2caddr;
	boolean readCommand(uint8_t reg, uint8_t *redata, uint8_t redata_length);
	void writeCommand(uint8_t reg, uint8_t *cmd, uint8_t length);
	void writeByte(uint8_t reg, uint8_t cmd);
};
#endif