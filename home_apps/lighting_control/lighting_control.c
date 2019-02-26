/**	Do Ngoc Nhuan, 22/02/2019
 *	lighting control application that includes automatic mode, light sensor monitoring, light state forcing.
 */
 
#include "lighting_control.h"

APP_TIMER_DEF(m_auto_timer_id);
static auto_mode_config_t auto_mode_config;
static lighting_control_info_t lighting_control_info;
static lc_get_info_event_callback_t m_registered_callback;
static bool auto_mode_is_on;

void lc_light_on(void)
{
	if (GPIO_LOGIC_LEVEL_LIGHT_ON==1)	{
		nrf_gpio_pin_set(GPIO_LIGHT_RELAY);
	} else	{
		nrf_gpio_pin_clear(GPIO_LIGHT_RELAY);
	}
}

void lc_light_off(void)
{
	if (GPIO_LOGIC_LEVEL_LIGHT_ON==1)	{
		nrf_gpio_pin_clear(GPIO_LIGHT_RELAY);
	} else	{
		nrf_gpio_pin_set(GPIO_LIGHT_RELAY);
	}
}

bool lc_light_is_on(void)
{
	bool state;
	state = (nrf_gpio_pin_out_read(GPIO_LIGHT_RELAY)==1) ? true : false;
	state = (GPIO_LOGIC_LEVEL_LIGHT_ON==1) ? state : !state;
	return state;
}

static void lc_light_sensor_init(void)
{
	ret_code_t err_code;
	err_code = isl29023_init(lc_isl29023_callback);
	APP_ERROR_CHECK(err_code);
}

void lc_lighting_control_init(lc_get_info_event_callback_t callback)
{
	ret_code_t err_code;
	lc_light_sensor_init();
	nrf_gpio_cfg_output(GPIO_LIGHT_RELAY);
	lc_light_off();
	
	auto_mode_is_on = false;
	m_registered_callback = callback;
	
	err_code = app_timer_create(&m_auto_timer_id, 
									APP_TIMER_MODE_REPEATED, auto_timer_timeout_handler);
	APP_ERROR_CHECK(err_code);
}

static void lc_isl29023_callback(uint16_t * lux)
{
	if (auto_mode_is_on)	{
		if (*lux<auto_mode_config.threshold)	{
			lc_light_on();
		} else	{
			lc_light_off();
		}
	}
	
	lighting_control_info.lux = *lux;
	lighting_control_info.state = (lc_light_is_on()) ? LIGHT_ON : LIGHT_OFF;
	
	m_registered_callback(&lighting_control_info);
}

void lc_start_get_info(void)
{
	ret_code_t err_code;
	err_code = isl29023_send_measuring_command();
	APP_ERROR_CHECK(err_code);
}

static void auto_timer_timeout_handler(void * p_context)
{
	lc_start_get_info();
}

void lc_auto_mode_set_cfg(auto_mode_config_t * auto_mode_CONFIG)
{
	if (auto_mode_CONFIG->set_as_default)	{
		auto_mode_config.threshold = AUTO_LUX_THRESHOLD_DEFAULT;
		auto_mode_config.interval = AUTO_INTERVAL;
		auto_mode_config.set_as_default = true;
	} else	{
		auto_mode_config.threshold = auto_mode_CONFIG->threshold;
		auto_mode_config.interval = auto_mode_CONFIG->interval;
		auto_mode_config.set_as_default = false;
	}
}

const auto_mode_config_t * lc_auto_mode_get_cfg(void)
{
	const auto_mode_config_t * auto_mode_Config;
	auto_mode_Config = &auto_mode_config;
	return auto_mode_Config;
}

void lc_set_on_auto_mode(void)
{
	ret_code_t err_code;
	err_code = app_timer_start(m_auto_timer_id, APP_TIMER_TICKS(auto_mode_config.interval), NULL);
	APP_ERROR_CHECK(err_code);
	auto_mode_is_on = true;
}

void lc_set_off_auto_mode(void)
{
	ret_code_t err_code;
	err_code = app_timer_stop(m_auto_timer_id);
	APP_ERROR_CHECK(err_code);
	auto_mode_is_on = false;
}
bool lc_auto_mode_is_on(void)
{
	return auto_mode_is_on;
}
