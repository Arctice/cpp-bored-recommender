#include "data_store.h"
#ifdef _WIN32
#include <WinSock2.h>
#endif

unique_ptr<redis> redis_connect(){
#ifdef _WIN32
	{
		WSAData wsinit;
		if(WSAStartup(MAKEWORD(2, 2), &wsinit)!=0){
			// iunno
		}
	}
#endif
	auto client = make_unique<redis>();
	client->connect();
	return client;
}
