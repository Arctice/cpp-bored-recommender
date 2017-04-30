#pragma once
#pragma comment(lib,"WS2_32")
#include <cpp_redis/cpp_redis>
using namespace std;
using redis = cpp_redis::redis_client;

shared_ptr<redis> redis_connect();