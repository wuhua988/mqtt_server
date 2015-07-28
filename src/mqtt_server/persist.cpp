//
//  persist.cpp
//  mqtt_server
//
//  Created by davad.di on 7/27/15.
//
//

#include "mqtt_server/persist.hpp"

namespace reactor
{
    int CPersist::write_str(std::string &str)
    {
	uint32_t len = str.length();
	ERROR_RETURN(this->write(&len, sizeof(uint32_t)), -1);
	ERROR_RETURN(this->write((void *)str.c_str(), len), -1);

	return 0;
    }

    int CPersist::write_buf(uint8_t *buf, uint32_t len)
    {
	ERROR_RETURN(this->write(buf, len), -1);
	return len;
    }

    int CPersist::write_int(uint32_t value)
    {
	ERROR_RETURN(this->write(&value, sizeof(uint32_t)), -1);

	return sizeof(value);
    }

    int CPersist::open(std::string str_file_name)
    {
	m_file_name = str_file_name;
	m_db_file = fopen(m_file_name.c_str(), "wb");
	if (m_db_file == nullptr)
	{
	    LOG_ERROR("Open file %s failed. errno %d, %s", m_file_name.c_str(), errno, strerror(errno));
	    return -1;
	}

	// write file_header

	return 0;
    }

    CPersist::CPersist()
    {
    }

    CPersist::CPersist(std::string file_name) : m_file_name(file_name)
    {
    }

    CPersist::~CPersist()
    {
	if (m_db_file != nullptr)
	{
	    fclose(m_db_file);
	}
    }


    int CPersist::store_db_info()
    {
	uint32_t chunk_len = sizeof(uint64_t);

	uint64_t last_msg_id = MSG_MEM_STORE->last_msg_id();
	ERROR_RETURN(this->write_chunk_info(CHUNK_TYPE::GLOBAL_INFO, chunk_len), -1);
	ERROR_RETURN(this->write((uint8_t *)&last_msg_id, sizeof(last_msg_id)), -1);

	return 0;
    }

    int CPersist::store_db_msg()
    {
	return 0;
    }

    int CPersist::store_client_info()
    {
	uint32_t chunk_len = 0;

	// std::unordered_map<std::string, CMqttClientContext_ptr>    m_client_msg;
	auto client_infos = CLIENT_ID_CONTEXT->m_client_msg;
	for (auto it = client_infos.begin(); it != client_infos.end(); it++)
	{
	    std::string client_id = it->first;
	    CMqttClientContext_ptr context = it->second;
	
	    ERROR_RETURN(this->write_chunk_info(CHUNK_TYPE::GLOBAL_INFO, chunk_len), -1);
	    // recorder offset 

	    ERROR_RETURN(this->write_str(client_id), -1);

	    // std::vector<CTopic>  & subcribe_topics()
	    auto topics = context->subcribe_topics();
	    uint32_t topic_size = topics.size();

	    ERROR_RETURN(this->write_int(topic_size), -1);

	    for (auto it1 = topics.begin(); it1 != topics.end(); it1++)
	    {
		std::string &topic_name = it1->topic_name();
		uint8_t     qos		= it1->qos();

		ERROR_RETURN(this->write_str(topic_name), -1);
		ERROR_RETURN(this->write_buf(&qos, sizeof(uint8_t)), -1);
	    }
	    
	    //std::list<CMbuf_ptr> & send_msg()
	    auto msgs = context->send_msg();
	    uint32_t msg_num = msgs.size();

	    ERROR_RETURN(this->write_int(msg_num), -1);
	    for (auto it2 = msgs.begin(); it2 != msgs.end(); it2++)
	    {
		uint64_t msg_id = (*it2)->msg_id();
		
		// store msg_id -> mbuf

		ERROR_RETURN(this->write_buf((uint8_t *)&msg_id, sizeof(uint64_t)), -1);	
	    }
	}

	return 0;
    }

    int CPersist::store_retain_msg()
    {
	return 0;
    }

    int CPersist::restore()
    {
	uint8_t chunk_type;
	uint8_t chunk_len;

	int res = 0;
	while(1)
	{
	    if (fread((void *)&chunk_type, 1, sizeof(chunk_type), m_db_file) != sizeof(chunk_type))
	    {
		return -1;
	    }

	    if (fread((void *)&chunk_len, 1, sizeof(chunk_len), m_db_file) != sizeof(chunk_len))
	    {
		return -1;
	    }

	    uint8_t  buf[65535];

	    if (fread((void *)buf, 1, chunk_len, m_db_file) != chunk_len)
	    {
		return -1;
	    }

	    switch ((CHUNK_TYPE)chunk_type)
	    {
		case CHUNK_TYPE::GLOBAL_INFO:
		    // res = MSG_MEM_STORE->restore((uint8_t *)buf, chunk_len); 
		    break;

		/*
		case CHUNK_TYPE::DB_MSG:
		    res = MSG_MEM_STORE->restore((uint8_t *)buf, chunk_len);
		    break;
		*/

		case CHUNK_TYPE::CLIENT_MSG:
		    // res = CLIENT_ID_CONTEXT->restore((uint8_t *)buf, chunk_len);
		    break;

		case CHUNK_TYPE::TOPIC_INFO:
		    // res = SUB_MGR->restore((uint8_t *)buf, chunk_len);
		    break;

		default:
		    LOG_ERROR("Unknown chunk type %d", chunk_type);
		    res = -1;
		    break;
	    }

	    if (res == -1)
	    {
		break;
	    }
	}

	return res;

    }

    int CPersist::store()
    {
	LOG_DEBUG("CPersist::store()");

	ERROR_RETURN(this->store_db_info(), -1);
	ERROR_RETURN(this->store_db_msg(), -1);
	ERROR_RETURN(this->store_client_info(), -1);
	ERROR_RETURN(this->store_retain_msg(), -1);

	return 0;
    }

    int CPersist::write_chunk_info(CHUNK_TYPE chunk_type, uint32_t chunk_len)
    {
	uint8_t type = (uint8_t)chunk_type;
	ERROR_RETURN(this->write((uint8_t *) &type, 1), -1);
	ERROR_RETURN(this->write((uint8_t *) &chunk_len, sizeof(uint32_t)), -1);  
    
	return 0;
    }

    int CPersist::write(void *buf, uint32_t len)
    {
	if (fwrite(buf, 1, len, m_db_file) !=  len)
	{
	    LOG_DEBUG("Write file %s failed. %d, %s", m_file_name.c_str(), errno, strerror(errno));
	    return -1;
	}

	return 0;
    }

} // end of namespace

