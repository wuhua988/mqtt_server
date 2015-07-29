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

class CMsgMemStore; 

const int MAX_DEFAULT_BUF_SIZE = 512;

class CMbuf: public std::enable_shared_from_this<CMbuf>
{
public:
    CMbuf(uint32_t size);
    CMbuf();
    ~CMbuf();

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

    
private:
    uint32_t           m_max_size;     /* 数据包大小 */
    uint8_t            *m_read_ptr;    /* read marker */
    uint8_t            *m_write_ptr;   /* write marker */
    uint8_t            *m_base_ptr;    /* start of buffer (const) */
    
    uint64_t           m_msg_id;

    CMsgMemStore       *m_mem_db = nullptr;
};

typedef std::shared_ptr<CMbuf> CMbuf_ptr;


#endif /* defined(____buf__) */
