#include "mbuf.hpp"
#include <string>

struct
{
    char      type:4;
    char      dup:1;
    char      qos:2;
    char      retain:1;
}TFixHeaderFlag;

struct
{
char:1;                            /**< unused */
     char    clean_session:1;
     char    will:1;
     char    will_qos:2;
     char    will_retail:1;
     char    password:1;
     char    username:1;
}TConnectFlag;

class CMqttPubMessage
{
    uint8_t     m_msg_cmd;
    uint8_t     m_qos;
    uint8_t     m_dup_flag;
    uint8_t     m_retain;

    uint32_t    m_remain_length;    // remain length

    uint16_t    m_msg_id;           // src msg id
    uint16_t    m_reserved;

    std::string m_str_topic;
    std::string m_payload;

    uint32_t    m_db_id;           //     time_t      m_timestamp;
};

class CTopic
{
    public:
	std::string m_topic_name;
	uint8_t     m_qos;
};

class CTMqttClientContext
{
    public:
	socket_t m_sock_id;

	std::string m_remote_addr;          // ip:port
	
	std::string m_protocal_name;
	char	    m_protocal_ver;

	std::string m_client_id;            // max len 23
	std::string m_user_name;
	std::string m_user_passwd;

	uint16_t    m_keep_alive_timer;
	uint16_t    m_last_msg_id;

	uint16_t    m_client_state;
	uint16_t    m_max_inflight_msgs;

	// time related
	time_t      m_last_msg_in;
	time_t      m_last_msg_out;
	time_t      m_ping;
	time_t      m_disconnect;

	time_t	    m_cli_msg_id; // last msg id from client
	time_t	    m_last_retry_check;
	bool	    m_clean_session;

	bool            m_will_flag;
	CMqttPubMessage m_will_msg;

	// std::list<CMbuf *> m_recv_msg_queue;
	// std::list<CMbuf *> m_send_msg_queue;
	// CMbuf   *m_recv_buf;                     

	std::list<CTopic>   m_subcribe_topics;

	//stat related struct
	uint64_t  m_qos_msg_count[3];           // qos0/1/2 
};

