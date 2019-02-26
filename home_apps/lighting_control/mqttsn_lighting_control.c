/**	Do Ngoc Nhuan, 23/02/2019
 *	MQTT-SN on OpenThread for lighting control application.
 */
 
#include "mqttsn_lighting_control.h"
 
mqttsn_client_t      m_client;                                       /**< An MQTT-SN client instance. */
static mqttsn_remote_t      m_gateway_addr;                                 /**< A gateway address. */
static uint8_t              m_gateway_id;                                   /**< A gateway ID. */
static mqttsn_connect_opt_t m_connect_opt;                                  /**< Connect options for the MQTT-SN client. */
uint16_t             m_msg_id           = 0;                         /**< Message ID thrown with MQTTSN_EVENT_TIMEOUT. */
static char                 m_client_id[]      = "light";     				/**< The MQTT-SN Client's ID. */
mqttsn_connect_steps_t connect_to_gateway_step;
static char                 m_topic_1_name[]   = "light/rcv_command";
static char                 m_topic_2_name[]   = "light/send_info";
mqttsn_topic_t       m_topic_1          =                            
{
    .p_topic_name = (unsigned char *)m_topic_1_name,
    .topic_id     = 0,
};
mqttsn_topic_t       m_topic_2          =                            
{
    .p_topic_name = (unsigned char *)m_topic_2_name,
    .topic_id     = 0,
};

/***************************************************************************************************
 * @section MQTT-SN handling application
 **************************************************************************************************/

static void lighting_control_evt_callback(lighting_control_info_t * p_lighting_control_info)
{
	ret_code_t err_code;
	static uint8_t str_Msg[10];
	
	memset(str_Msg, NULL, sizeof(str_Msg));
	make_msg_lc_light_info(p_lighting_control_info, str_Msg);

	if (connect_to_gateway_step == CONNECTED_SUCCESSFULLY)	{
		err_code = mqttsn_client_publish(&m_client, m_topic_2.topic_id, str_Msg, strlen((char *)str_Msg), &m_msg_id);
		APP_ERROR_CHECK(err_code);
	} else {
		NRF_LOG_INFO("Can't publish message because of non-connected gateway.");
	}
}

static void lc_str_handling_msg_to_send_ready_callback(uint8_t * msg_to_send, uint8_t length)
{
	ret_code_t err_code;
	
	if (connect_to_gateway_step == CONNECTED_SUCCESSFULLY)	{
		err_code = mqttsn_client_publish(&m_client, m_topic_2.topic_id, msg_to_send, length, &m_msg_id);
		APP_ERROR_CHECK(err_code);
	} else {
		NRF_LOG_INFO("Can't publish message because of non-connected gateway.");
	}
}

static void lighting_control_auto_cfg(void)
{
	auto_mode_config_t auto_mode_cfg;
	
	memset(&auto_mode_cfg, 0, sizeof(auto_mode_cfg));
	auto_mode_cfg.set_as_default = true;
	
	lc_auto_mode_set_cfg(&auto_mode_cfg);
}

/***************************************************************************************************
 * @section MQTT-SN
 **************************************************************************************************/
/**@brief Processes GWINFO message from a gateway.
 *
 * @details This function updates MQTT-SN Gateway information.
 *
 * @param[in]    p_event  Pointer to MQTT-SN event.
 */
static void gateway_info_callback(mqttsn_event_t * p_event)
{
    m_gateway_addr = *(p_event->event_data.connected.p_gateway_addr);
    m_gateway_id   = p_event->event_data.connected.gateway_id;
	connect_to_gateway_step = CONNECTING_STEP;
	mqttsn_lc_connect_to_gateway(connect_to_gateway_step);
}


/**@brief Processes CONNACK message from a gateway.
 *
 * @details This function launches the topic registration procedure if necessary.
 */
static void connected_callback(void)
{
    LEDS_ON(BSP_LED_3_MASK);
	
	connect_to_gateway_step = REGISTERING_TOPIC_1_STEP;
	mqttsn_lc_connect_to_gateway(connect_to_gateway_step);
}


/**@brief Processes DISCONNECT message from a gateway. */
static void disconnected_callback(void)
{
    LEDS_OFF(BSP_LED_3_MASK);
}

/**@brief Processes REGACK message from a gateway.
 *
 * @param[in] p_event Pointer to MQTT-SN event.
 */
static void regack_callback(mqttsn_event_t * p_event)
{
	if (connect_to_gateway_step == REGISTERING_TOPIC_1_STEP)	{
		m_topic_1.topic_id = p_event->event_data.registered.packet.topic.topic_id;
		NRF_LOG_INFO("MQTT-SN event: Topic 1 has been registered with ID: %d.\r\n",
                 p_event->event_data.registered.packet.topic.topic_id);
		
		connect_to_gateway_step = REGISTERING_TOPIC_2_STEP;
		mqttsn_lc_connect_to_gateway(connect_to_gateway_step);
		
	} else if (connect_to_gateway_step == REGISTERING_TOPIC_2_STEP){
		m_topic_2.topic_id = p_event->event_data.registered.packet.topic.topic_id;
		NRF_LOG_INFO("MQTT-SN event: Topic 2 has been registered with ID: %d.\r\n",
                 p_event->event_data.registered.packet.topic.topic_id);
		
		connect_to_gateway_step = SUBSCRIBING_TOPIC_1_STEP;
		mqttsn_lc_connect_to_gateway(connect_to_gateway_step);
	} else {
		NRF_LOG_ERROR("Error happen in registering topic.");
	}
}

static void subscribed_callback(void)
{
	connect_to_gateway_step = CONNECTED_SUCCESSFULLY;
	NRF_LOG_INFO("MQTT-SN client for lighting control connected to gateway.");
}

/**@brief Processes data published by a broker.
 *
 * @details This function recognize control command.
 */
static void received_callback(mqttsn_event_t * p_event)
{
	if (p_event->event_data.published.packet.topic.topic_id == m_topic_1.topic_id)
    {
        NRF_LOG_INFO("MQTT-SN event: Content to subscribed topic received.\r\n");
		
        recognize_lc_control_command(p_event->event_data.published.p_payload,
											p_event->event_data.published.payload_len);
    }
    else
    {
        NRF_LOG_INFO("MQTT-SN event: Content to unsubscribed topic received. Dropping packet.\r\n");
    }
}

/**@brief Processes retransmission limit reached event. */
static void timeout_callback(mqttsn_event_t * p_event)
{
    NRF_LOG_INFO("MQTT-SN event: Timed-out message: %d. Message ID: %d.\r\n",
                  p_event->event_data.error.msg_type,
                  p_event->event_data.error.msg_id);
	
	mqttsn_client_uninit(&m_client);
	mqttsn_lc_init();
	mqttsn_lc_connect_to_gateway(STARTING_CONNECT_PROCESS);
}


/**@brief Processes results of gateway discovery procedure. */
static void searchgw_timeout_callback(mqttsn_event_t * p_event)
{
    NRF_LOG_INFO("MQTT-SN event: Gateway discovery result: 0x%x.\r\n", p_event->event_data.discovery);
}


/**@brief Function for handling MQTT-SN events. */
static void mqttsn_evt_handler(mqttsn_client_t * p_client, mqttsn_event_t * p_event)
{
    switch(p_event->event_id)
    {
        case MQTTSN_EVENT_GATEWAY_FOUND:
            NRF_LOG_INFO("MQTT-SN event: Client has found an active gateway.\r\n");
            gateway_info_callback(p_event);
            break;

        case MQTTSN_EVENT_CONNECTED:
            NRF_LOG_INFO("MQTT-SN event: Client connected.\r\n");
            connected_callback();
            break;

        case MQTTSN_EVENT_DISCONNECT_PERMIT:
            NRF_LOG_INFO("MQTT-SN event: Client disconnected.\r\n");
            disconnected_callback();
            break;

        case MQTTSN_EVENT_REGISTERED:
            NRF_LOG_INFO("MQTT-SN event: Client registered topic.\r\n");
            regack_callback(p_event);
            break;
		
        case MQTTSN_EVENT_SUBSCRIBED:
            NRF_LOG_INFO("MQTT-SN event: Client subscribed to topic 1.\r\n");
			subscribed_callback();
            break;
        
		case MQTTSN_EVENT_RECEIVED:
            NRF_LOG_INFO("MQTT-SN event: Client received content.\r\n");
            received_callback(p_event);
            break;
		
        case MQTTSN_EVENT_PUBLISHED:
            NRF_LOG_INFO("MQTT-SN event: Client has successfully published content.\r\n");
            break;

        case MQTTSN_EVENT_TIMEOUT:
            NRF_LOG_INFO("MQTT-SN event: Retransmission retries limit has been reached.\r\n");
            timeout_callback(p_event);
            break;

        case MQTTSN_EVENT_SEARCHGW_TIMEOUT:
            NRF_LOG_INFO("MQTT-SN event: Gateway discovery procedure has finished.\r\n");
            searchgw_timeout_callback(p_event);
            break;

        default:
            break;
    }
}

void mqttsn_lc_disconnect_to_gateway(void)
{
	ret_code_t err_code;
	if (mqttsn_client_state_get(&m_client) == MQTTSN_CLIENT_CONNECTED)
	{
		err_code = mqttsn_client_disconnect(&m_client);
		if (err_code != NRF_SUCCESS)
		{
			NRF_LOG_ERROR("DISCONNECT message could not be sent. Error: 0x%x\r\n", err_code);
		}
	} else	{
		NRF_LOG_INFO("MQTT-SN client disconnected.");
	}
}
void mqttsn_lc_connect_to_gateway(mqttsn_connect_steps_t step)
{
	ret_code_t err_code;
	
	if ((mqttsn_client_state_get(&m_client) == MQTTSN_CLIENT_CONNECTED) 
			&& (step <= CONNECTING_STEP))
	{
		NRF_LOG_INFO("MQTT-SN client connected.");
		step = REGISTERING_TOPIC_1_STEP;
	}
	
	switch (step)	{
		case SEARCHING_GATEWAY_STEP:
		{
			err_code = mqttsn_client_search_gateway(&m_client, SEARCH_GATEWAY_TIMEOUT);
            if (err_code != NRF_SUCCESS)
            {
                NRF_LOG_ERROR("SEARCH GATEWAY message could not be sent. Error: 0x%x\r\n", err_code);
            }
		}
			break;
		
		case CONNECTING_STEP:
		{
			err_code = mqttsn_client_connect(&m_client, &m_gateway_addr, m_gateway_id, &m_connect_opt);
			if (err_code != NRF_SUCCESS)
			{
				NRF_LOG_ERROR("CONNECT message could not be sent. Error: 0x%x\r\n", err_code);
			}
		}
			break;
		
		case REGISTERING_TOPIC_1_STEP:
		{
			err_code = mqttsn_client_topic_register(&m_client,
													 m_topic_1.p_topic_name,
													 strlen(m_topic_1_name),
													 &m_msg_id);
			if (err_code != NRF_SUCCESS)
			{
				NRF_LOG_ERROR("REGISTER message for topic 1 could not be sent. Error code: 0x%x\r\n", err_code);
			}
		}
			break;
		
		case REGISTERING_TOPIC_2_STEP:
		{
			err_code = mqttsn_client_topic_register(&m_client,
													 m_topic_2.p_topic_name,
													 strlen(m_topic_2_name),
													 &m_msg_id);
			if (err_code != NRF_SUCCESS)
			{
				NRF_LOG_ERROR("REGISTER message for topic 2 could not be sent. Error code: 0x%x\r\n", err_code);
			}
		}
			break;
		
		case SUBSCRIBING_TOPIC_1_STEP:
		{
			err_code = mqttsn_client_subscribe(&m_client, 
												m_topic_1.p_topic_name, 
												strlen(m_topic_1_name),
												&m_msg_id);
			if (err_code != NRF_SUCCESS)
			{
				NRF_LOG_ERROR("SUBSCRIBE message for topic 1 could not be sent.\r\n");
			}
		}
			break;
		
		default:
			break;
	}// end switch..case..
}

/**@brief Initializes MQTT-SN client's connection options.
 */
static void connect_opt_init(void)
{
    m_connect_opt.alive_duration = MQTTSN_DEFAULT_ALIVE_DURATION,
    m_connect_opt.clean_session  = MQTTSN_DEFAULT_CLEAN_SESSION_FLAG,
    m_connect_opt.will_flag      = MQTTSN_DEFAULT_WILL_FLAG,
    m_connect_opt.client_id_len  = strlen(m_client_id),

    memcpy(m_connect_opt.p_client_id, (unsigned char *)m_client_id, m_connect_opt.client_id_len);
}

/**@brief Function for initializing the MQTTSN client.
 */
static void mqttsn_lc_init(void)	{
	uint32_t err_code = mqttsn_client_init(&m_client,
                                           MQTTSN_DEFAULT_CLIENT_PORT,
                                           mqttsn_evt_handler,
                                           thread_ot_instance_get());
    APP_ERROR_CHECK(err_code);
    connect_opt_init();
	connect_to_gateway_step = SEARCHING_GATEWAY_STEP;
}

/**@brief Function for initializing the MQTTSN client & lighting control application.
 */
void mqttsn_lighting_control_init(void)
{
	mqttsn_lc_init();
	
	lc_lighting_control_init(lighting_control_evt_callback);
	lc_str_handling_set_callback(lc_str_handling_msg_to_send_ready_callback);
	lighting_control_auto_cfg();
}
