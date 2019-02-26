/**	Do Ngoc Nhuan, 24/02/2019
 *	Module for handling string data receive from user and preparing string data to send to user.
 */
#ifndef LC_STRING_HANDLING_H
#define LC_STRING_HANDLING_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "mem_manager.h"

#include "lighting_control.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"

#define MAXIMUM_COMMAND_STRING_LENGTH	20 //byte
#define LC_GET							1
#define LC_SET							2
#define LC_MODE							3
#define LC_STATE						4
#define LC_ON							5
#define LC_OFF							6
#define LC_AUTO							7
#define LC_MANUAL						8
#define LC_END							9

/**@brief Function for encrypting command string received on topic 2 (topic for user command).
 *
 * @details		Words in the string will be encrypt to integer numbers that are stored in a result array.
 *
 * @param[in]    p_command   		Pointer to payload.
 * @param[in]    cmdStrLen   		Length of payload.
 * @param[in]    encrypted_command	Pointer to array storing results of this encrypting process.
 */
bool encrypt_mqttsn_lc_command(const uint8_t * p_command, uint8_t cmdStrLen, uint8_t * encrypted_command);

void convert_int_to_string(uint32_t num, uint8_t * str);

/**@brief Function for making ASCII message that will be used to publish on topic 2 (information topic).
 *
 * @param[in]    p_lighting_control_info   		Pointer to infomation of lighting device.
 * @param[in]    str_msg   			 			Pointer to memory area that store result message.
 */
void ascii_msg_mqttsn_lc_info(const lighting_control_info_t * p_lighting_control_info, uint8_t * str_msg);

#endif // LC_STRING_HANDLING_H
