/**	Do Ngoc Nhuan, 22/02/2019
 *	isl29032 driver for nRF5 SDK version 15.0.0 using app timer.
 */
 
#ifndef __ISL_29023_SENSOR_H__
#define __ISL_29023_SENSOR_H__

#include "app_timer.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"

#include "nrf_log.h"

/* TWI instance ID. */
#define ISL29023_TWI_INSTANCE_ID     1

/* TWI0 definition for light sensor. */
#define ISL29023_SCL	NRF_GPIO_PIN_MAP(1,8)
#define ISL29023_SDA	NRF_GPIO_PIN_MAP(1,9)
#define ISL29023_ADDR   0x44U			//refer page 6 of ISL29023 datasheet.

/* ISL29023 Register Address */
#define CMD1			0x00
#define CMD2			0x01
#define DATA_L			0x02
#define DATA_H			0x03
#define INT_LT_L		0x04
#define INT_LT_H		0x05
#define INT_HT_L		0x06
#define INT_HT_H		0x07

/* Mode for ISL29023. */
#define CMD1_ONE_READ 	0x20U			//refer p.8&p.9 of ISL29023 datasheet.
#define CMD2_16BIT		0x03U
/* Integration time of ADC in ISL29032*/
#define ISL29023_TIME_16BITS	90	//in milisecond
#define ISL29023_TIME_12BITS	6	//in milisecond
#define ISL29023_TIME_8BITS		352	//in microsecond
#define ISL29023_TIME_4BITS		22	//in microsecond

/**@brief ISL29023 read light intensity process complete event callback function type.
 *
 * @details     Upon an event in read light intensity process complete, this callback function will be called 
 *				to notify the application about the event.
 *
 * @param[in]   p_isl29023_lux Pointer to light intensity variable.
 */
typedef void (* isl29023_event_callback_t)(uint16_t * p_isl29023_lux);

/**
 * @brief Write data to ISL29023 sensor.
 *
 * @param[in]   p_data           Pointer to data to write.
 * @param[in]   length           Length of data to write [in byte uint].
 */
static uint32_t isl29023_write(uint8_t *p_data, uint8_t length);

/**
 * @brief Function for starting TWI read process to archive data from ISL29023 sensor.
 */
static uint32_t isl29023_twi_read_lux_start(void);

/**@brief       Send a measuring command to isl29023.
 * @details     The function sends a measuring command through TWI. Continuously, it start an app timer
 *				to wait for measuring complete.
 */
uint32_t isl29023_send_measuring_command(void);

/**@brief       Convert data read from sensor into lux uint.
 */
static void convert_data_into_lux(void);

static void isl29023_twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context);

/**@brief       Function for initializing TWI to communicate to light sensor.
 */
static uint32_t isl29023_twi_init(void);

/**@brief       Function for initializing isl29023 light sensor.
 *
 * @details     The function initializes TWI, assigns callback, creates an app timer.
 *
 * @param[in]   callback           Function to be called when button press/event is detected.
 */
uint32_t isl29023_init(isl29023_event_callback_t callback);

#endif /*__ISL_29023_SENSOR_H__*/
