#include "config.h"
#include "data_store.h"
#include "recommendations.h"
#include <chrono>
#include <thread>

// lazy
string jsonify(const map<media_type, media_values>& recs){
	string json = "{\"anime\": [";

	auto anime_recs_count = min(
			recs.at(ANIME).size(),
			(size_t)MAX_RECS);
	for(int i = 0; i<anime_recs_count; ++i){
		auto id = recs.at(ANIME)[i].first.second;
		json += to_string(id);
		if(i<anime_recs_count-1)
			json += ", "s;
	}

	json += "], \"manga\": ["s;

	auto manga_recs_count = min(
		recs.at(MANGA).size(),
		(size_t)MAX_RECS);
	for(int i = 0; i<manga_recs_count; ++i){
		auto id = recs.at(MANGA)[i].first.second;
		json += to_string(id);
		if(i<manga_recs_count-1)
			json += ", "s;
	}

	json += "]}"s;

	return json;
}

void recommendations_job
(const string& job, redis& data_store, const vector<user_scores>& all_ratings)
{
	auto recs = recommendations(job, data_store, all_ratings);
	
	auto json = jsonify(recs);

	cout<<"done "<<job<<endl;
	data_store.hset(RECOMMENDATIONS_RAW, job, json);
	data_store.rpop(RECOMMENDATION_QUEUE);
	data_store.lpush(NEW_RECOMMENDATIONS, {job});
	data_store.sync_commit();
}

int main(){
	string queue = RECOMMENDATION_QUEUE;

	auto r = redis_connect();

	vector<user_scores> all_ratings;
	for(auto name : all_usernames(*r)){
		auto scores = get_scores(name, *r);
		all_ratings.push_back(scores);
	}

	while(true){
		string job;
		r->lindex(queue, -1,
			[&](cpp_redis::reply& reply){
				if(reply.is_null())
					job = "";
				else
					job = reply.as_string();
			});
		r->sync_commit();

		if(job.empty()){
			this_thread::sleep_for(chrono::seconds(2));
			continue;
		}

		cout<<"recommendation queue tail: ";

		r->lrange(queue, -8, -1, [](cpp_redis::reply& reply){
			if(reply.is_null()) cout<<"[]";
			else{
				for(const auto& elem : reply.as_array())
					cout<<elem.as_string()+", ";
			}
			cout<<endl;
		});
		r->sync_commit();
		cout<<"processing "<<job<<endl;
		
		recommendations_job(job, *r, all_ratings);
	}

	cin.get();
}
