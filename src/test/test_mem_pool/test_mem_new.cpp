#include <stdlib.h>
#include <stdio.h>

#include <mbuf.hpp>

int main()
{
    for(unsigned int i=0; i<0x5fffff; i++)
    {
	CMbuf *p = new CMbuf();
	delete p;
    }

    return 0;
} 
