#include "mbuf.hpp"
#include <list>

void print(CMbuf &buffer)
{
     fprintf(stderr, "base_ptr 0x%0x, read_ptr 0x%x, write_ptr 0x%x, lenght %d, max_size %d\n",      
	      buffer.base_ptr(), 
	      buffer.read_ptr(),
	      buffer.write_ptr(), 
	      buffer.length(), 
	      buffer.end_ptr() -  buffer.base_ptr());
}

std::list<CMbuf_ptr> send_queue;

int send(CMbuf_ptr &buf)
{
    LOG_DEBUG("Begin in send refer_count %d", buf.use_count());
    send_queue.push_back(buf);
    LOG_DEBUG("After in send refer_count %d", buf.use_count());      
    
}

int main()
{
    CLoggerMgr logger("log4cplus.properties");

    {
	std::shared_ptr<CMbuf> buffer = make_shared<CMbuf>();
	//std::shared_ptr<CMbuf> buf_ptr = buffer->Copy();
	//auto buf_ptr2 = buffer->shared_from_this();
	LOG_DEBUG("Before call send buffer refer count %d", buffer.use_count());

	send(buffer);
	LOG_DEBUG("After call send buffer refer count %d", buffer.use_count());   
    }

    {
	auto buf_ptr = send_queue.front();
	send_queue.pop_front();

	 LOG_DEBUG("After call pop front refer count %d", buf_ptr.use_count());
    }

    LOG_DEBUG("Reached the end");


    return 0;
/*
    print(*buffer);

    buffer->copy((const uint8_t*)"hello", 5);

    print(*buffer);   

    char buf[2048];

    memcpy(buf, buffer->read_ptr(), buffer->length());

    print(*buffer);

    buffer->read_ptr(5);

    print(*buffer);

    buffer->copy((const uint8_t *)buf, 1025);

    print(*buffer);
    */

}
