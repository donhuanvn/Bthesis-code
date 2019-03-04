/**	Do Ngoc Nhuan, 03/03/2019
 *	si7021 sensor driver for nRF5 SDK version 15.0.0 using app timer.
 */
#ifndef __SI_7021_SENSOR_H__
#define __SI_7021_SENSOR_H__

#include "app_timer.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

/* TWI instance ID. */
#define SI7021_TWI_INSTANCE_ID     0

/* TWIx definition for light sensor. */
#define SI7021_SCL			21
#define SI7021_SDA			23
#define SI7021_ADDR   		0x40U			//refer page 18 of Si7021-A20 datasheet.

/* Si7021 I2C Command (refer p.18 of Si7021-A20 datasheet)*/
#define MRH_HMM				0xE5U	//Measure Relative Humidity, Hold Master Mode
#define MRH_NHMM			0xF5U	//Measure Relative Humidity, No Hold Master Mode
#define MT_HMM				0xE3U	//Measure Temperature, Hold Master Mode
#define MT_NHMM				0xF3U	//Measure Temperature, No Hold Master Mode
#define RTVFPRHM			0xE0U	//Read Temperature Value from Previou Humidity Measure
#define SI7021_RST			0xFEU	//Reset
#define WRH_T_UR_1			0xE6U	//Write RH/T User Register 1
#define RRH_T_UR_1			0xE7U	//Read RH/T User Register 1
#define WHCR				0x51U	//Write Heater Control Register
#define RHCR				0x11U	//Read Heater Control Register
#define REID_1ST_BYTE_1		0xFAU	//Read Electronic ID 1st Byte (the left byte)
#define REID_1ST_BYTE_2		0x0FU	//Read Electronic ID 1st Byte (the right byte)
#define REID_2ND_BYTE_1		0xFCU	//Read Electronic ID 2nd Byte (the left byte)
#define REID_2ND_BYTE_2		0xC9U	//Read Electronic ID 2nd Byte (the right byte)
#define RFR_1				0x84U	//Read Firmware Revision (the left byte)
#define RFR_2				0xB8U	//Read Firmware Revision (the right byte)

#define SI7021_MAXIMUM_TIME_FOR_MEASURING	50 //miliseconds, refer p.5 of Si7021-A20 datasheet

typedef struct si7021_data_t
{
	float humidity;
	float temperature;
} si7021_data_t;

typedef void (* si7021_event_callback_t)(si7021_data_t * p_si7021_data);

void si7021_init(si7021_event_callback_t callback);
void si7021_start_read_process(void);

#endif /*__ISL_29023_SENSOR_H__*/
