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
		case CHUNK_TYPE::DB_MSG:
		    res = MSG_MEM_STORE->restore((uint8_t *)buf, chunk_len);
		    break;

		case CHUNK_TYPE::CLIENT_MSG:
		    res = CLIENT_ID_CONTEXT->restore((uint8_t *)buf, chunk_len);
		    break;

		case CHUNK_TYPE::TOPIC_INFO:
		    res = SUB_MGR->restore((uint8_t *)buf, chunk_len);
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
	// 1. db.store
	MSG_MEM_STORE->store(this);

	// 2. client_msg store
	CLIENT_ID_CONTEXT->store(this);

	// 3. topic store
	SUB_MGR->store(this);

	return 0;
    }

    int CPersist::write(uint8_t *buf, uint32_t len)
    {
	if (fwrite(buf, 1, len, m_db_file) !=  len)
	{
	    LOG_DEBUG("Write file %s failed. %d, %s", m_file_name.c_str(), errno, strerror(errno));
	    return -1;
	}

	return 0;
    }

} // end of namespace

