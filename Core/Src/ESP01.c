/*
 * ESP01.c
 *
 *  Created on: 9 wrz 2021
 *      Author: igur
 */


#include "ESP01.h"
#include "usart.h"

extern const char API_Key[];
extern uint8_t UART_RX_val;
extern RTC_TimeTypeDef rtc_time;

static ESP_status ESP8266_wait_for_msg(ESP8266_t* ESP, char* message)
{
	ESP_RESPOND_FLAG = 1;

	do
	{

		Parser_clean_string(&ESP->ESP_RX_Buff, &ESP->ESP_RX_msg_to_parsed);
		if(Parser_simple_parse(message, &ESP->ESP_RX_msg_to_parsed))
		{
			RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
			RB_Flush(&ESP->ESP_RX_msg_to_parsed);
			memset(ESP->ESP_RX_Buff.buffer, 0x00, sizeof(ESP->ESP_RX_Buff.buffer[0]) * BUFFER_SIZE);
			memset(ESP->ESP_RX_msg_to_parsed.buffer, 0x00, sizeof(ESP->ESP_RX_msg_to_parsed.buffer[0]) * BUFFER_SIZE);

			ESP_RESPOND_FLAG = 0;
		}

		else if(Parser_simple_parse("ERROR", &ESP->ESP_RX_msg_to_parsed))
		{
			RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
			RB_Flush(&ESP->ESP_RX_msg_to_parsed);
			memset(ESP->ESP_RX_Buff.buffer, 0x00, sizeof(ESP->ESP_RX_Buff.buffer[0]) * BUFFER_SIZE);
			memset(ESP->ESP_RX_msg_to_parsed.buffer, 0x00, sizeof(ESP->ESP_RX_msg_to_parsed.buffer[0]) * BUFFER_SIZE);
			return ESP_NOK;
		}

		else if(Parser_simple_parse("FAIL", &ESP->ESP_RX_msg_to_parsed))
		{
			RB_Flush(&ESP->ESP_RX_Buff); // clean buffer before next received message
			RB_Flush(&ESP->ESP_RX_msg_to_parsed);
			memset(ESP->ESP_RX_Buff.buffer, 0x00, BUFFER_SIZE);
			memset(ESP->ESP_RX_msg_to_parsed.buffer, 0x00, BUFFER_SIZE);
			return ESP_NOK;
		}

	}while(ESP_RESPOND_FLAG);

	return ESP_OK;
}


static ESP_status ESP8266_is_TCP_disconnected(ESP8266_t* ESP)
{
	if(ESP->ESP_RX_Buff.BUFFER_EMPTY_FLAG)
	{
		return ESP_NOK;
	}

	Parser_clean_string(&ESP->ESP_RX_Buff, ESP->MessageReceive);

	if(!(Parser_simple_parse("CLOSED", ESP->MessageReceive)))
	{
		return ESP_NOK;
	}

	return ESP_OK;

}


static ESP_status ESP8266_Disconnect(ESP8266_t* ESP)
{
	uint8_t message[36];
	uint8_t length;
	length = sprintf((char*)message, "AT+CWQAP\r\n");
	UART_send_message((char*)message, length);
	return ESP_OK;
}


ESP_status ESP8266_EnterLightSleep(ESP8266_t* ESP, uint8_t GPIO_wakeup, uint8_t gpio_trigg_level)
{
	uint8_t message[26];
	uint8_t length;

	if(gpio_trigg_level)
	{
		HAL_GPIO_WritePin(ESP_wakeup_GPIO_Port, ESP_wakeup_Pin, GPIO_PIN_RESET);
	}
	else HAL_GPIO_WritePin(ESP_wakeup_GPIO_Port, ESP_wakeup_Pin, GPIO_PIN_SET);

	length = sprintf((char*)message, "AT+SLEEPWKCFG=2,%d,%d\r\n", GPIO_wakeup, gpio_trigg_level);
	UART_send_message((char*)message, length);
	ESP8266_wait_for_msg(ESP, "OK");

	length = sprintf((char*)message, "AT+SLEEP=2\r\n");
	UART_send_message((char*)message, length);
	ESP8266_wait_for_msg(ESP, "OK");

	return ESP_OK;
}

ESP_status ESP8266_EnterDeepSleep(ESP8266_t* ESP, uint16_t sleep_seconds)
{
	uint8_t message[26];
	uint8_t length;

	uint32_t sleep_ms = sleep_seconds * 1000;

	length = sprintf((char*)message, "AT+GSLP=%d\r\n", sleep_ms);
	UART_send_message((char*)message, length);
	ESP8266_wait_for_msg(ESP, "OK");
}



static ESP_status ESP8266_Connect_To_Router(ESP8266_t* ESP)
{
	uint8_t message[36];
	uint8_t length;
	length = sprintf((char*)message, "AT+CWJAP=\"%s\",\"%s\"\r\n", ESP->SSID, ESP->PSWD);
	UART_send_message((char*)message, length);

	if(ESP8266_wait_for_msg(ESP, "IPOK") == ESP_NOK)
	{
		return ESP_NOK;
	}
	return ESP_OK;
}

ESP_status ESP8266_SetMode(ESP8266_t* ESP, ESP_mode mode)
{
	switch(mode)
	{
	case STATION:
		UART_send_string("AT+CWMODE=1\r\n"); // Set WiFi mode to station mode
		break;

	case ACCESS_POINT:
		UART_send_string("AT+CWMODE=2\r\n"); // Set WiFi mode to access point
		break;

	case AP_STATION:
		UART_send_string("AT+CWMODE=3\r\n"); // Set WiFi mode to station mode + AP mode
		break;
	}

	if(!(ESP8266_wait_for_msg(ESP, "OK")))
	{
		return ESP_NOK;
	}

	return ESP_OK;
}


ESP_status ESP8266_Init(ESP8266_t* ESP, char* SSID, char* PSWD, ESP_mode Mode)
{
	ESP->SSID = SSID;
	ESP->PSWD = PSWD;

	ESP8266_Disconnect(ESP);
	ESP8266_wait_for_msg(ESP, "OK");
	RingBuffer_Init(&ESP->ESP_RX_Buff);
	RingBuffer_Init(&ESP->ESP_RX_msg_to_parsed);

	// TCP client connection config:
	// 1. Set WiFi mode
	if(ESP8266_SetMode(ESP, Mode) == ESP_NOK)
	{
		return ESP_NOK;
	}
	// 2. Connect to a router
	while((ESP8266_Connect_To_Router(ESP)) == ESP_NOK)
	{
		// if ESP couldn't connect to WiFi, entry lower mode for specified time and try to reconnect
		Entry_LowPowerMode(ESP_RECONNECTING_TO_WIFI_TIME_INTERVAL);
	}

	return ESP_OK;
}

ESP_status ESP8266_SetConnectionMode(ESP8266_t* ESP, ESP_ConnectionMode mode)
{
	switch(mode)
	{
	case SINGLE_CONNECTION:
		UART_send_string("AT+CIPMUX=0\r\n");
		break;
	case MULTIPLE_CONNECTION:
		UART_send_string("AT+CIPMUX=1\r\n");
		break;
	}

	if(ESP8266_wait_for_msg(ESP, "OK") == ESP_NOK)
	{
		return ESP_NOK;
	}

	return ESP_OK;
}


ESP_status ESP8266_Connect_TCP(ESP8266_t* ESP, char* Target_IP, char* PORT, ESP_ConnectionMode mode)
{
	ESP8266_SetConnectionMode(ESP, mode);

	// prepare message
	uint8_t message[128];
	uint8_t length;
	length = sprintf((char*)message, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", Target_IP, PORT);
	UART_send_message((char*)message, length);

	if(ESP8266_wait_for_msg(ESP, "OK") == ESP_NOK)
	{
		return ESP_NOK;
	}

	return ESP_OK;
}

ESP_status ESP8266_Disconnect_TCP(ESP8266_t* ESP)
{
	UART_send_string("AT+CIPCLOSE\r\n");
	ESP8266_wait_for_msg(ESP, "CLOSED");
	return ESP_OK;
}


ESP_status ESP8266_TS_Send_Data_MultiField(ESP8266_t* ESP, uint8_t number_of_fields, uint16_t data_buffer[])
{
	while(ESP8266_Connect_TCP(ESP, "184.106.153.149", "80", SINGLE_CONNECTION) == ESP_NOK)
	{
		if(ESP8266_Connect_To_Router(ESP) == ESP_NOK)
		{
			return ESP_NOK;
		}
	}

	char cipsend_buff[25] = {0};
	char field_buff[35] = {0};
	char message[128] = {0};

	// prepare message
	sprintf(message, "GET /update?api_key=%s", API_Key);
	for(int i = 1; i < number_of_fields + 1; i++)
	{
		sprintf(field_buff, "&field%d=%u", i, data_buffer[i - 1]);
		strcat(message, field_buff);
	}
	strcat(message, "\r\n");

	// send data length information
	sprintf(cipsend_buff, "AT+CIPSEND=%d\r\n", strlen(message));
	UART_send_message(cipsend_buff, strlen(cipsend_buff));

	if(ESP8266_wait_for_msg(ESP, "OK") == ESP_NOK)
	{
		ESP8266_Disconnect_TCP(ESP);
		return ESP_NOK;
	}

	UART_send_message(message, strlen(message)); // send data

	// if TCP isn't closed
	if(ESP8266_is_TCP_disconnected(ESP) != ESP_OK)
	{
		ESP8266_Disconnect_TCP(ESP);
	}

	ESP->ESP8266_status = ESP_OK;
	return ESP_OK;
}

