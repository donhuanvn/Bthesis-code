/**	Do Ngoc Nhuan, 24/02/2019
 *	Module for handling string data receive from user and preparing string data to send to user.
 */

#include "lc_string_handling.h"

bool encrypt_mqttsn_lc_command(const uint8_t * p_command, uint8_t cmdStrLen, uint8_t * encrypted_command)
{
	char * array_command = (char *)nrf_malloc(MAXIMUM_COMMAND_STRING_LENGTH*sizeof(char));
	char * saveptr;
	//initialized array_command for this process.
	memset(array_command, 0, 20*sizeof(char));
	//initialized result array.
	memset(encrypted_command, 0, 4);
	//copy received payload to another array before handling. 
	strncpy(array_command, (char *)p_command, cmdStrLen);
	
	//++++++++++++++++FIRST++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	
	//get the first word
	strtok_r(array_command, "_", &saveptr);
	//check the first word
	if (strcmp(array_command, "get") == 0)	{
		*(encrypted_command + 0) = LC_GET;
	} else if (strcmp(array_command, "set") == 0)	{
		*(encrypted_command + 0) = LC_SET;
	} else {
		goto rcv_command_error;
	}
	//check length of the remain string after the frist strtok_r
	if (strlen(saveptr)==0) goto encrypted_complete;

	//++++++++++++++++SECOND+++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	
	//remain string is pointed by saveptr. Update to array_command for next process.
	array_command = saveptr;
	//get the second word
	strtok_r(array_command, "_", &saveptr);
	//check the second word
	if (strcmp(array_command, "mode")==0)	{
		*(encrypted_command + 1) = LC_MODE;
	} else if (strcmp(array_command, "state")==0)	{
		*(encrypted_command + 1) = LC_STATE;
	} else {
		goto rcv_command_error;
	}

	//check length of the remain string after the second strtok_r
	if (strlen(saveptr)==0) goto encrypted_complete;
	
	//++++++++++++++++THIRD++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	
	//remain string is pointed by saveptr. Update to array_command for next process.
	array_command = saveptr;
	//get the third word
	strtok_r(array_command, "_", &saveptr);
	//check the third word
	if (strcmp(array_command, "on")==0)	{
		*(encrypted_command + 2) = LC_ON;
	} else if (strcmp(array_command, "off")==0)	{
		*(encrypted_command + 2) = LC_OFF;
	} else if (strcmp(array_command, "auto")==0)	{
		*(encrypted_command + 2) = LC_AUTO;
	} else if (strcmp(array_command, "manual")==0)	{
		*(encrypted_command + 2) = LC_MANUAL;
	} else	{
		goto rcv_command_error;
	}
	//check length of the remain string after the third strtok_r
	if (strlen(saveptr)==0)	{
		goto encrypted_complete;
	} else	{
		goto rcv_command_error;
	}
	
	//++++++++++++++++++END++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	
	encrypted_complete:
		nrf_free(array_command);
		return true;
	
	rcv_command_error:
		NRF_LOG_ERROR("Can't recognize received command!");
		nrf_free(array_command);
		return false;
}

void convert_int_to_string(uint32_t num, uint8_t * str)	{
	uint8_t c, lenStr = 0;
	
	do	{
		c = (num%10)+48;
		num /= 10;
		*(str+lenStr) = c;
		lenStr++;
	}while (num != 0);
	
	*(str+lenStr) = NULL; //NULL: end of string
	
	for (uint8_t i=0;i<lenStr/2;i++)	{
		c = *(str+i);
		*(str+i) = *(str+(lenStr-1)-i);
		*(str+(lenStr-1)-i) = c;
	}
}

void ascii_msg_mqttsn_lc_info(const lighting_control_info_t * p_lighting_control_info, uint8_t * str_msg)
{
	convert_int_to_string(p_lighting_control_info->lux, str_msg);
	
	if (p_lighting_control_info->state == LIGHT_ON)	{
		strcat((char *)str_msg, "_on");
	} else {
		strcat((char *)str_msg, "_off");
	}
}
