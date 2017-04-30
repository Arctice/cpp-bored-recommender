#include "data_store.h"
#include <WinSock2.h>

std::shared_ptr<redis> redis_connect(){
	{
		WSAData wsinit;
		if(WSAStartup(MAKEWORD(2, 2), &wsinit)!=0){
			// iunno
		}
	}
	auto client = std::make_unique<redis>();
	client->connect();
	return client;
}
