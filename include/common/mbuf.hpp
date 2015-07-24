#ifndef ____buf__
#define ____buf__

/*
 * idea from twoemproxy
 *
 * mbuf header is at the tail end of the mbuf. This enables us to catch
 * buffer overrun early by asserting on the magic value during get or
 * put operations
 *
 *   <------------- mbuf_chunk_size ------------->
 *   +-------------------------------------------+
 *   |       mbuf data          |  mbuf header   |
 *   |     (mbuf_offset)        | (struct mbuf)  |
 *   +-------------------------------------------+
 *   ^           ^        ^     ^^
 *   |           |        |     ||
 *   \           |        |     |\
 *   mbuf->start \        |     | mbuf->end (one byte past valid bound)
 *                mbuf->pos     \
 *                        \      mbuf
 *                        mbuf->last (one byte past valid byte)
 *
 */

#include <memory>
#include "reactor/define.hpp" 

const int MAX_DEFAULT_BUF_SIZE = 512;

class CMbuf: public std::enable_shared_from_this<CMbuf>
{
public:
    CMbuf(uint32_t size)
    {
	LOG_TRACE_METHOD(__func__);      
        m_max_size = size;
        m_base_ptr = new uint8_t[size]; // later from mem pool
        
        m_read_ptr = m_base_ptr;
        m_write_ptr = m_base_ptr;
    }
    
    CMbuf()
    {
	LOG_TRACE_METHOD(__func__); 
        m_max_size = MAX_DEFAULT_BUF_SIZE; // default size len
        m_base_ptr = new uint8_t[m_max_size];
        
        m_read_ptr = m_base_ptr;
        m_write_ptr = m_base_ptr;
    }
    
    ~CMbuf()
    {
	LOG_TRACE_METHOD(__func__);
	delete []m_base_ptr;
    }

    std::shared_ptr<CMbuf> copy()
    {
	return shared_from_this();
    }

    // get read ptr
    uint8_t * read_ptr()
    {
        return m_read_ptr;
    }
    
    // set write ptr skip offset
    void read_ptr(uint32_t n)
    {
        if (m_read_ptr + n > m_base_ptr + m_max_size)
        {
            return;
        }
        
        m_read_ptr += n;
    }
    
    // get read ptr
    uint8_t * write_ptr()
    {
        return m_write_ptr;
    }
    
    // set write ptr skip offset
    void write_ptr(uint32_t n)
    {
        if (m_write_ptr + n > m_base_ptr + m_max_size)
        {
            return;
        }
        
        m_write_ptr += n;
    }
    
    // copy data to buf, and  adjust offset
    int copy(const uint8_t *buf, int len)
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
    uint8_t * base_ptr()
    {
        return m_base_ptr;
    }
    
    // get end ptr
    uint8_t * end_ptr()
    {
        return m_base_ptr + m_max_size;
    }
    
    uint32_t length()
    {
        return uint32_t(m_write_ptr - m_read_ptr);
    }
    
    void reset()
    {
        m_read_ptr = m_base_ptr;
        m_write_ptr = m_base_ptr;
    }

    uint32_t available_buf()
    {
        LOG_DEBUG("In available_buf, max_size %d, m_base_ptr 0x%p, m_write_ptr 0x%p",
				    m_max_size, m_base_ptr, m_write_ptr);

	return uint32_t(m_max_size + m_base_ptr - m_write_ptr);
    }

    uint32_t max_size()
    {
	return m_max_size;
    }
    
    void msg_id(uint64_t msg_id)
    {
        m_msg_id = msg_id;
    }
    
    uint64_t msg_id()
    {
        return m_msg_id;
    }

    
private:
    uint32_t           m_max_size;     /* 数据包大小 */
    uint8_t            *m_read_ptr;    /* read marker */
    uint8_t            *m_write_ptr;   /* write marker */
    uint8_t            *m_base_ptr;    /* start of buffer (const) */
    
    uint64_t           m_msg_id;
};

typedef std::shared_ptr<CMbuf> CMbuf_ptr;

#endif /* defined(____buf__) */
