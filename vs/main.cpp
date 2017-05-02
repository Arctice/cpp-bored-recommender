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
		cout<< z[ANIME][i].first.second <<": "<<z[ANIME][i].second<<endl;

	cout<<"\nmanga\n";
	for(int i = 0; i<10; i++)
		cout<<z[MANGA][i].first.second <<": "<<z[MANGA][i].second<<endl;

	cin.get();
}