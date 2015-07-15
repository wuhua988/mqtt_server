#include "mbuf.hpp"

#ifndef __mbuf_mem_pool_h_
#define __mbuf_mem_pool_h_
#include <list>

/*template later*/
class CMemPool
{
    public:
	CMemPool(int size)
	{
	    for (int i = 0; i < size; i++)
	    {
		m_list_mbuf.push_back(make_shared<CMbuf>());   
	    }
	}

	 CMemPool()
	 {
	     int size = 1024;
	     for (int i = 0; i < size; i++)
	     {
		 m_list_mbuf.push_back(make_shared<CMbuf>());  
	     }
	 }

	 ssize_t size()
	 {
	     return m_list_mbuf.size();
	 }

	~CMemPool()
	{
	    m_list_mbuf.clear();

	    /*
	    auto it = m_ist_mbuf.begin();

	    if (auto it = m_list_mbuf.begin(); it != m_list_mbuf.end(); it++)
	    {
		delete *it;
	    }
	    */
	}

	CMbuf_ptr get()
	{
	    if (!m_list_mbuf.empty())
	    {
		LOG_DEBUG("m_list_mbuf is not embpty, size %d", (uint32_t)m_list_mbuf.size());
		
		CMbuf_ptr buf = m_list_mbuf.front();
		m_list_mbuf.pop_front();

		static int count = 0;
		
		if (buf.get() == nullptr)
		{
		    LOG_DEBUG("Get return point null, count %d", count++);
		}

		return buf;
	    }

	    LOG_DEBUG("m_list_mbuf is embpty, size %d, create new mbuf now", (uint32_t)m_list_mbuf.size());
	    return make_shared<CMbuf>();
	}

	void unget(CMbuf_ptr buf)
	{
	    
	    LOG_DEBUG("In mem pool unget, use_count  %d", (int32_t)buf.use_count());
	    
	    buf->reset();
	    m_list_mbuf.push_back(buf);
	}

	
    private:
	std::list<CMbuf_ptr> m_list_mbuf;
};

#endif

