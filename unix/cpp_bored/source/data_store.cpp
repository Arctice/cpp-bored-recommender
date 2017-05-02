#include "data_store.h"
#ifdef WIN32
#include <WinSock2.h>
#endif

std::shared_ptr<redis> redis_connect(){
	{
        #ifdef WIN32
		WSAData wsinit;
		if(WSAStartup(MAKEWORD(2, 2), &wsinit)!=0){
			// iunno
		}
        #endif
	}
	auto client = std::make_unique<redis>();
	client->connect();
	return client;
}
