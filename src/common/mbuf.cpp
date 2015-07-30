#include "common/mbuf.hpp"
#include "common/msg_mem_store.hpp" 

CMbuf::CMbuf(uint32_t size)
{
    LOG_TRACE_METHOD(__func__);      
    m_max_size = size;
    m_base_ptr = new uint8_t[size]; // later from mem pool

    m_read_ptr = m_base_ptr;
    m_write_ptr = m_base_ptr;

    m_msg_id = 0;
}

CMbuf::CMbuf()
{
    LOG_TRACE_METHOD(__func__); 
    m_max_size = MAX_DEFAULT_BUF_SIZE; // default size len
    m_base_ptr = new uint8_t[m_max_size];

    m_read_ptr = m_base_ptr;
    m_write_ptr = m_base_ptr;

    m_msg_id = 0;
}

CMbuf::~CMbuf()
{
    LOG_TRACE_METHOD(__func__);

    if (m_msg_id && m_mem_db != nullptr)
    {
	m_mem_db->del_msg(m_msg_id);
    }

    delete []m_base_ptr;
}

std::shared_ptr<CMbuf> CMbuf::copy()
{
    return shared_from_this();
}

// get read ptr
uint8_t * CMbuf::read_ptr()
{
    return m_read_ptr;
}

// set write ptr skip offset
void CMbuf::read_ptr(uint32_t n)
{
    if (m_read_ptr + n > m_base_ptr + m_max_size)
    {
	return;
    }

    m_read_ptr += n;
}

// get read ptr
uint8_t * CMbuf::write_ptr()
{
    return m_write_ptr;
}

// set write ptr skip offset
void CMbuf::write_ptr(uint32_t n)
{
    if (m_write_ptr + n > m_base_ptr + m_max_size)
    {
	return;
    }

    m_write_ptr += n;
}

// copy data to buf, and  adjust offset
int CMbuf::copy(const uint8_t *buf, int len)
{
    if (m_write_ptr - m_base_ptr + len > m_max_size)
    {
	return -1;
    }

    memcpy(m_write_ptr, buf, len);

    m_write_ptr += len;

    return 0;
}

// get base ptr
uint8_t * CMbuf::base_ptr()
{
    return m_base_ptr;
}

// get end ptr
uint8_t * CMbuf::end_ptr()
{
    return m_base_ptr + m_max_size;
}

uint32_t CMbuf::length()
{
    return uint32_t(m_write_ptr - m_read_ptr);
}

void CMbuf::reset()
{
    m_read_ptr = m_base_ptr;
    m_write_ptr = m_base_ptr;
}

uint32_t CMbuf::available_buf()
{
    LOG_DEBUG("In available_buf, max_size %d, m_base_ptr 0x%p, m_write_ptr 0x%p",
	    m_max_size, m_base_ptr, m_write_ptr);

    return uint32_t(m_max_size + m_base_ptr - m_write_ptr);
}

uint32_t CMbuf::max_size()
{
    return m_max_size;
}

void CMbuf::msg_id(uint64_t msg_id, bool regist_to_db)
{
    m_msg_id = msg_id;

    if (regist_to_db)
    {
	m_mem_db = MSG_MEM_STORE;
	
	// add to mem store
	if (msg_id && m_mem_db != nullptr)
	{
	    m_mem_db->add_msg(msg_id, this);
	}
    }
}

uint64_t CMbuf::msg_id()
{
    return m_msg_id;
}

