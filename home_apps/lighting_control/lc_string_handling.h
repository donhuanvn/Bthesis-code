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

#define MAXIMUM_NUMBER_OF_CALLBACK			2
#define MAX_STRING_SEND_LENGTH				20
#define MAX_NUM_OF_WORD_IN_CMD				10
#define LENGTH_CODE_ARRAY					10
#define MAXIMUM_COMMAND_STRING_LENGTH		20 //byte
#define LC_THIS_IS_NUMBER					1
#define LC_GET_WORD							2
#define LC_SET_WORD							3
#define LC_MODE_WORD						4
#define LC_STATE_WORD						5
#define LC_ON_WORD							6
#define LC_OFF_WORD							7
#define LC_AUTO_WORD						8
#define LC_MANUAL_WORD						9
#define LC_NOTHING							0

static bool lc_string_find_word(const char * p_word, int8_t * p_result);
static bool lc_string_convert_to_uint(const char * p_num, int8_t * p_char_result, int32_t * p_num_result);

typedef void (* lc_str_handling_callback_t)(uint8_t * p_msg_to_send, uint8_t length);

/**@brief Function for encrypting command string received on topic 2 (topic for user command).
 *
 * @details		Words in the string will be encrypt to integer numbers that are stored in a result array.
 *				@Notes if encrypted_char_cmd[i] = LC_THIS_IS_NUMBER, please get number from encrypted_num_cmd[i].
 * @param[in]    p_command   		Pointer to payload.
 * @param[in]    cmdStrLen   		Length of payload.
 * @param[out]   encrypted_char_cmd	Pointer to array storing results of this encrypting process.
 * @param[out]   encrypted_num_cmd	Pointer to array storing number results of this encrypting process.
 */
bool encrypt_mqttsn_lc_command(const uint8_t * p_command, uint8_t cmdStrLen, 
									int8_t * encrypted_char_cmd, int32_t * encrypted_num_cmd);

void convert_int_to_string(uint32_t num, uint8_t * str);

/**@brief Function for making ASCII message that will be used to publish on topic 2 (information topic).
 *
 * @param[in]    p_lighting_control_info   		Pointer to infomation of lighting device.
 * @param[in]    str_msg   			 			Pointer to memory area that store result message.
 */
void make_msg_lc_light_info(const lighting_control_info_t * p_lighting_control_info, uint8_t * str_msg);
void recognize_lc_control_command(uint8_t * p_cmd, uint8_t cmd_length);
bool lc_str_handling_set_callback(lc_str_handling_callback_t callback);
#endif // LC_STRING_HANDLING_H
