#include "stdio.h"
#include "flow.h"

int main()
{
	uint8 packet[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
					  0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
					  0xaa, 0xbb, 
					  
					  0xdd,0xdd,0xdd,0xdd,
					  0xee,0xee,0xee,0xee,
					  0xaa,
					  0xa1,
					  0xcc,0xdd,
					  0xee,0xff,0xee,
					  0x66,
					  
					  0x66,
					  0xff,0xff,
					  0xaa,0xbb,0xcc,0xdd,
					  0x55,0x55
					  };
	struct
	{
		struct miniflow mf;
		uint64_t buf[FLOW_U64S];
	} key;
	miniflow_extract(packet, &key.mf);
	return 0;
}