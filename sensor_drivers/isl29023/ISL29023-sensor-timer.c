/**	Do Ngoc Nhuan, 22/02/2019
 *	isl29032 driver for nRF5 SDK version 15.0.0 using app timer.
 */

#include "ISL29023-sensor-timer.h"

APP_TIMER_DEF(m_timer_id);
/* Indicates if operation on TWI has ended. */
static volatile bool xfer_tx_done = false;
static volatile bool xfer_rx_done = false;
static volatile bool reading_lux_on_process = false;
static isl29023_event_callback_t m_registered_callback = NULL;
uint16_t isl29023_lux;
static uint8_t p_read_byte[2];
/* TWI instance. */
static const nrf_drv_twi_t isl29023_m_twi = NRF_DRV_TWI_INSTANCE(ISL29023_TWI_INSTANCE_ID);

static uint32_t isl29023_write(uint8_t *p_data, uint8_t length)
{
	ret_code_t err_code = NRF_SUCCESS;
	xfer_tx_done = false;

	err_code = nrf_drv_twi_tx(&isl29023_m_twi, ISL29023_ADDR, p_data, length, false);
	if (err_code != NRF_SUCCESS)	{
		return err_code;
	}
	
	return err_code;
}

static uint32_t isl29023_twi_read_lux_start(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	static uint8_t reg_addr = DATA_L;	//start at DATA_L 
	
	xfer_tx_done = false;
	xfer_rx_done = false;
		
	err_code = nrf_drv_twi_tx(&isl29023_m_twi, ISL29023_ADDR, &reg_addr, 1, true);
	if (err_code != NRF_SUCCESS)	{
		return err_code;
	}
	
	nrf_delay_ms(2);
	
	err_code = nrf_drv_twi_rx(&isl29023_m_twi, ISL29023_ADDR, p_read_byte, 2);	//DATA_L, DATA_H
	if (err_code != NRF_SUCCESS)	{
		return err_code;
	}
	//wait for reading process complete.
	return err_code;
}

static void isl29023_timer_timeout_handler(void * p_context)
{
	ret_code_t err_code = NRF_SUCCESS;
	
	err_code = isl29023_twi_read_lux_start();
	APP_ERROR_CHECK(err_code);
}

uint32_t isl29023_send_measuring_command(void)
{
	ret_code_t err_code = NRF_SUCCESS;
	static uint8_t w_data[2] = {CMD1, CMD1_ONE_READ}; 					//writing_data
	
	if (reading_lux_on_process)	{
		return err_code;
	}
	
	reading_lux_on_process = true;
	
	err_code = isl29023_write(w_data, sizeof(w_data)); 					//(data to write, length of this data)
	if (err_code != NRF_SUCCESS)	{
		return err_code;
	}
	
	err_code = app_timer_start(m_timer_id, APP_TIMER_TICKS(ISL29023_TIME_16BITS), NULL);
	if (err_code != NRF_SUCCESS)	{
		return err_code;
	}
	
	return err_code;
}

static void convert_data_into_lux(void)
{
	uint16_t	i_lux;
	float 		real_lux;
	
	i_lux = p_read_byte[1];
	i_lux <<= 8;
	i_lux |= p_read_byte[0];
	
	real_lux = i_lux*1000/65536;								// For this formula, refer at page 10 of ISL29023 datasheet 

	isl29023_lux = (uint16_t) real_lux;
}

/**
 * @brief ISL29023 TWI events handler.
 */
static void isl29023_twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
//	uint32_t err_code = NRF_SUCCESS;
	
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_TX) 
			{
				xfer_tx_done = true;
			}
			else if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)	//event for reading completed
            {
				xfer_rx_done = true;
				
				if (reading_lux_on_process)	{
					reading_lux_on_process = false;
					
					convert_data_into_lux();
					m_registered_callback(&isl29023_lux);
				}
            }
            break;
        default:
            break;
    }
}
/**
 * @brief Two wire interface (I2C) initialization.
 */
static uint32_t isl29023_twi_init(void)
{
    ret_code_t err_code = NRF_SUCCESS;

    const nrf_drv_twi_config_t isl29023_twi_config = {
       .scl                = ISL29023_SCL,
       .sda                = ISL29023_SDA,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&isl29023_m_twi, &isl29023_twi_config, isl29023_twi_handler, NULL);
    if (err_code != NRF_SUCCESS)	{
		return err_code;
	}

    nrf_drv_twi_enable(&isl29023_m_twi);
	
	return err_code;
}

uint32_t isl29023_init(isl29023_event_callback_t callback)
{
	ret_code_t err_code = NRF_SUCCESS;
	
	err_code = isl29023_twi_init();
	if (err_code != NRF_SUCCESS)	{
		return err_code;
	}
	m_registered_callback = callback;	//callback for lux data reading successful.
	err_code = app_timer_create(&m_timer_id, APP_TIMER_MODE_SINGLE_SHOT, isl29023_timer_timeout_handler);
	
	return err_code;
}
