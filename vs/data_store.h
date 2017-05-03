#pragma once
#ifdef _WIN32
#pragma comment(lib,"WS2_32")
#endif

#include <cpp_redis/cpp_redis>
using namespace std;
using redis = cpp_redis::redis_client;

unique_ptr<redis> redis_connect();