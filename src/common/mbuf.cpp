#include "common/mbuf.hpp"
#include "common/msg_mem_store.hpp" 


CMbuf::CMbuf(uint32_t size)
{
    LOG_TRACE_METHOD(__func__);
    init(size);
}

CMbuf::CMbuf()
{
    LOG_TRACE_METHOD(__func__);
    init(MAX_DEFAULT_BUF_SIZE);
}

int CMbuf::init(uint32_t size)
{
    m_data.size = size;
    m_data.ptr = (const char *)new uint8_t[size];  // later from mem pool

    m_read_pos = 0;
    m_write_pos = 0;
    m_msg_id = 0;

    m_msg_type = MSG_DATA;
    
    m_time = time(0);
    return 0;
}

void CMbuf::regist_mem_store(CMsgMemStore *mem_store)
{
    m_mem_db = mem_store;

    if (m_msg_id && m_mem_db != nullptr)
    {
	m_mem_db->add_msg(m_msg_id, this);
    }
}

CMbuf::~CMbuf()
{
    LOG_TRACE_METHOD(__func__);

    if (m_msg_id && m_mem_db != nullptr)
    {
	m_mem_db->del_msg(m_msg_id);
    }


    if (m_data.ptr != nullptr)
    {
	uint8_t *p = (uint8_t *)m_data.ptr;
	delete []p;

	m_data.ptr = nullptr;
    }
    else
    {
	LOG_DEBUG("!!!!!! Maybe double free");
    }

}

std::shared_ptr<CMbuf> CMbuf::copy()
{
    return shared_from_this();
}

// get read ptr
uint8_t * CMbuf::read_ptr()
{
    return (uint8_t *)(m_data.ptr + m_read_pos);
}

// set write ptr skip offset
void CMbuf::read_ptr(uint32_t n)
{
    if (m_read_pos + n > m_data.size)
    {
	return;
    }

    m_read_pos += n;
}

// get read ptr
uint8_t * CMbuf::write_ptr()
{
    return (uint8_t *)(m_data.ptr + m_write_pos);;
}

// set write ptr skip offset
void CMbuf::write_ptr(uint32_t n)
{
    if (m_write_pos + n > m_data.size)
    {
	return;
    }

    m_write_pos += n;
}

// copy data to buf, and  adjust offset
int CMbuf::copy(const uint8_t *buf, int len)
{
    if (m_write_pos + len > m_data.size)
    {
	return -1;
    }

    memcpy((void *)(m_data.ptr + m_write_pos), buf, len);

    m_write_pos += len;

    return 0;
}

// get base ptr
uint8_t * CMbuf::base_ptr()
{
    return (uint8_t *)m_data.ptr;
}

// get end ptr
uint8_t * CMbuf::end_ptr()
{
    return (uint8_t *)(m_data.ptr + m_data.size);
}

uint32_t CMbuf::length()
{
    return uint32_t(m_write_pos - m_read_pos);
}

void CMbuf::reset()
{
    m_read_pos = 0;
    m_write_pos = 0;
    // m_msg_id = 0;
}

uint32_t CMbuf::available_buf()
{
    return uint32_t(m_data.size - m_write_pos);
}

uint32_t CMbuf::max_size()
{
    return m_data.size;
}

void CMbuf::msg_id(uint64_t msg_id)
{
    m_msg_id = msg_id;
}

uint64_t CMbuf::msg_id()
{
    return m_msg_id;
}

void CMbuf::msg_type(uint32_t type)
{
    m_msg_type = type;
}

uint32_t CMbuf::msg_type()
{
    return m_msg_type;
}

uint32_t CMbuf::time()
{
    return m_time;
}

void hex_dump(uint8_t *buf, uint32_t len)
{
    LOG_DEBUG("0000 ");
    for (uint i = 0; i < len; i++)
    {
	if (i && ((i%16) == 0))
	{
	    LOG_DEBUG("\n%04x ", i);
	}

	LOG_DEBUG("%02X ", buf[i]);
    }
}

