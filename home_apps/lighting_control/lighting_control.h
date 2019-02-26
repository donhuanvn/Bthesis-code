/**	Do Ngoc Nhuan, 22/02/2019
 *	lighting control application that includes automatic mode, light sensor monitoring, light state forcing.
 */

#ifndef LIGHTING_CONTROL_H
#define LIGHTING_CONTROL_H

#include "boards.h"
#include "app_timer.h"
#include "app_error.h"
#include "nrfx_gpiote.h"
#include "ISL29023-sensor-timer.h"

#include "nrf_log.h"

#define LIGHT_ON					1
#define LIGHT_OFF					0
#define GPIO_LOGIC_LEVEL_LIGHT_ON	0
#define GPIO_LIGHT_RELAY			15
#define AUTO_LUX_THRESHOLD_DEFAULT	100
#define AUTO_INTERVAL				5000

typedef struct lighting_control_info_t
{
	uint16_t lux;
	uint32_t state;
} lighting_control_info_t;
typedef struct auto_mode_config_t
{
	uint16_t threshold;
	uint32_t interval;			//miliseconds
	bool set_as_default;
} auto_mode_config_t;
typedef void (* lc_get_info_event_callback_t)(lighting_control_info_t * p_lighting_control_info);

void lc_light_on(void);
void lc_light_off(void);
bool lc_light_is_on(void);
static void lc_light_sensor_init(void);
void lc_lighting_control_init(lc_get_info_event_callback_t callback);
static void lc_isl29023_callback(uint16_t * lux);
void lc_start_get_info(void);
static void auto_timer_timeout_handler(void * p_context);
void lc_auto_mode_set_cfg(auto_mode_config_t * auto_mode_config);
const auto_mode_config_t * lc_auto_mode_get_cfg(void);
void lc_set_on_auto_mode(void);
void lc_set_off_auto_mode(void);
bool lc_auto_mode_is_on(void);
#endif // LIGHTING_CONTROL_H
