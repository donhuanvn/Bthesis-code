/**	Do Ngoc Nhuan, 03/03/2019
 *	si7021 sensor driver for nRF5 SDK version 15.0.0 using app timer.
 */
 
#include "si7021-sensor-apptimer.h"

/* Indicates if operation on TWI has ended. */
static volatile bool m_xfer_done = false;
/* Indicates if reading data sensor (RH & Temperature) operation has ended. */
static bool read_proc_done = false;
/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(SI7021_TWI_INSTANCE_ID);
/* Si7021 Commands. */
static uint8_t measure_RH_cmd = MRH_NHMM;
static uint8_t read_temp_cmd = RTVFPRHM;

/* Buffer for samples read from Si7021 sensor. */
static uint8_t raw_data[4];
/* Application timer used for delaying. */
APP_TIMER_DEF(m_timer_id);
/* Si7021 sensor value updated event callback. */
static si7021_event_callback_t m_registered_callback = NULL;
/* Si7021 sensor value. */
static si7021_data_t si7021_data;
/* stage of reading data sensor operation. */
static uint8_t stage;

static void convert_raw_data_to_real_data(void)
{
	uint16_t RH_code = (raw_data[0]<<8) | raw_data[1];
	uint16_t Temp_code = (raw_data[2]<<8) | raw_data[3];
	
	si7021_data.humidity = ((float)125.0 * (float)RH_code) / (float)65536.0 - (float)6.0;			//refer p.21 of Si7021-A20 datasheet
	si7021_data.temperature = ((float)175.72 * (float)Temp_code) / (float)65536.0 - (float)46.85;	//refer p.22 of Si7021-A20 datasheet
}

/**
 * @brief TWI events handler.
 */
static void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
						switch (p_event->xfer_desc.type)
						{
							case NRF_DRV_TWI_XFER_RX:
								if (read_proc_done) {
									convert_raw_data_to_real_data();
									m_registered_callback(&si7021_data);
								}
								break;
							case NRF_DRV_TWI_XFER_TX:
								/*<   >*/
								break;
							default:
								break;
						}
            m_xfer_done = true;
            break;
        default:
            break;
    }
}

static void si7021_read_on_process(void)
{
	ret_code_t err_code;

	switch (stage)	{
		case 0:
				err_code = nrf_drv_twi_tx(&m_twi, SI7021_ADDR, &measure_RH_cmd, sizeof(measure_RH_cmd), true);
				APP_ERROR_CHECK(err_code);
				err_code = app_timer_start(m_timer_id, APP_TIMER_TICKS(30), NULL);
				APP_ERROR_CHECK(err_code);
			break;
		case 1:
				err_code = nrf_drv_twi_rx(&m_twi, SI7021_ADDR, &raw_data[0], 2);
				APP_ERROR_CHECK(err_code);
				err_code = app_timer_start(m_timer_id, APP_TIMER_TICKS(10), NULL);
				APP_ERROR_CHECK(err_code);
			break;
		case 2:
				err_code = nrf_drv_twi_tx(&m_twi, SI7021_ADDR, &read_temp_cmd, sizeof(read_temp_cmd), true);
				APP_ERROR_CHECK(err_code);
				err_code = app_timer_start(m_timer_id, APP_TIMER_TICKS(5), NULL);
				APP_ERROR_CHECK(err_code);
			break;
		case 3:
				err_code = nrf_drv_twi_rx(&m_twi, SI7021_ADDR, &raw_data[2], 2);
				APP_ERROR_CHECK(err_code);
				read_proc_done = true;
			break;
		default:
			break;
	}
	
	stage++;
}

void si7021_start_read_process(void)
{
	stage = 0;
	si7021_read_on_process();
}

static void si7021_timeout_handler(void * p_context)
{	
	si7021_read_on_process();
}

/**
 * @brief TWI initialization.
 */
static void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_Si7021_config = {
       .scl                = SI7021_SCL,
       .sda                = SI7021_SDA,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_Si7021_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);
		
    nrf_drv_twi_enable(&m_twi);
}
void si7021_init(si7021_event_callback_t callback)
{
	ret_code_t err_code = NRF_SUCCESS;
	
	twi_init();
	m_registered_callback = callback;
	
	err_code = app_timer_create(&m_timer_id, APP_TIMER_MODE_SINGLE_SHOT, si7021_timeout_handler);
	APP_ERROR_CHECK(err_code);
}
