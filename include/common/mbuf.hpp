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

#include "msgpack/msgpack.hpp"

class CMsgMemStore; 

const int MAX_DEFAULT_BUF_SIZE = 512;
const uint32_t MSG_DATA = 1;
const uint32_t MSG_PUBLISH = 2;

class CMbuf: public std::enable_shared_from_this<CMbuf>
{
public:
    CMbuf(uint32_t size);
    CMbuf();
    ~CMbuf();

    void regist_mem_store(CMsgMemStore *mem_store);

    int init(uint32_t size);

    std::shared_ptr<CMbuf> copy();

    // get read ptr
    uint8_t * read_ptr();
    
    // set write ptr skip offset
    void read_ptr(uint32_t n);
    
    // get read ptr
    uint8_t * write_ptr();
    
    // set write ptr skip offset
    void write_ptr(uint32_t n);
    
    // copy data to buf, and  adjust offset
    int copy(const uint8_t *buf, int len);
    
    // get base ptr
    uint8_t * base_ptr();
    
    // get end ptr
    uint8_t * end_ptr();
    
    uint32_t length();
    
    void reset();

    uint32_t available_buf();

    uint32_t max_size();
    
    void msg_id(uint64_t msg_id);   
    uint64_t msg_id();

    void msg_type(uint32_t msg_type);
    uint32_t msg_type();
    
private:
    msgpack::type::raw_ref  m_data;
    
    uint32_t		m_msg_type;
    uint32_t            m_read_pos;    /* read marker */
    uint32_t            m_write_pos;   /* write marker */
    uint64_t            m_msg_id;

    CMsgMemStore       *m_mem_db = nullptr;
    
public:
    MSGPACK_DEFINE(m_data, m_msg_type, m_read_pos, m_write_pos, m_msg_id);    
};

class CMbuf_tmp
{
    public:
	msgpack::type::raw_ref  m_data;
	uint32_t	    m_msg_type;
	uint32_t            m_read_pos;
	uint32_t            m_write_pos;
	uint64_t            m_msg_id;
    public:
	MSGPACK_DEFINE(m_data, m_msg_type, m_read_pos, m_write_pos, m_msg_id); 
};

typedef std::shared_ptr<CMbuf> CMbuf_ptr;


#endif /* defined(____buf__) */
