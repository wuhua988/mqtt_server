#ifndef _xml_config_h__
#define _xml_config_h__

#include "common/tinyxml2.h"
#include "reactor/define.hpp"
#include "common/singleton.hpp"

#include <string>


class CXMLConfig
{
    public:

	CXMLConfig(const char *file_name) : m_file_name(file_name)
    {
    }

	CXMLConfig(std::string &str_file_name) : m_file_name(str_file_name)
    {
    }

	CXMLConfig()
	{
	}

	int open(std::string &str_file_name)
	{
	    m_file_name = str_file_name;
	    return this->open();
	}
	int open(const char *file_name)
	{
	    m_file_name = file_name;
	    return this->open();
	}


	int open();
	void print();
	int read_node_text(tinyxml2::XMLElement *elment, const char *name, std::string &str_value);

	/*attribute*/
	GETSETVAR(std::string, file_name);
	GETSETVAR(std::string, log_conf_name);
	GETSETVAR(std::string, server_listen_ip);
	GETSETVAR(std::string, parent_server_ip);
	GETSETVAR(std::string, parent_user_name);
	GETSETVAR(std::string, parent_topic_name);
	GETSETVAR(std::string, db_file_name);


	GETSETVAR(uint16_t, server_listen_port);
	GETSETVAR(uint16_t, parent_server_port);
	GETSETVAR(uint16_t, parent_keep_alive);

	GETSETVAR(uint32_t, thread_number);
	GETSETVAR(uint32_t, flush_interval);
	GETSETVAR(uint32_t, max_idle_timeout);
};

typedef CSingleton<CXMLConfig> XMLConfig;

#define CONFIG  XMLConfig::instance()

#endif


