#include "data_store.h"
#include "recommendations.h"
#include <chrono>
using namespace std::chrono;

int main(){
	auto r = redis_connect();


	auto start = high_resolution_clock::now();

	auto z = recommendations("rainlife", *r);

	auto finish = high_resolution_clock::now();
	cout<<duration_cast<milliseconds>(finish-start).count()<<endl;

	cout<<"anime\n";
	for(int i = 0; i<10; i++)
		cout<<z.first[i].first<<": "<<z.first[i].second<<endl;

	cout<<"\nmanga\n";
	for(int i = 0; i<10; i++)
		cout<<z.second[i].first<<": "<<z.second[i].second<<endl;

	cin.get();
}