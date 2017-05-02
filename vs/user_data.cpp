#include "user_data.h"
#include <sstream>
#include <set>

vector<pair<int, int>> parse_scores(const string& stored_string){
	vector<pair<int, int>> scores;

	stringstream parse_stream(stored_string);
	if(!stored_string.empty())
	while(!parse_stream.eof()){
		// this code is probably broke as hell
		int id, score;
		parse_stream.get();  // open paren
		parse_stream >> id;
		parse_stream.get();  // comma
		parse_stream >> score;
		parse_stream.get(); parse_stream.get();  // close paren, comma
		scores.push_back({id, score});
	}

	return scores;
}

user_scores get_scores(const string& name, redis& data_store, bool only_rated){
	user_scores scores{name};

	for(auto& type:
		{make_pair(MANGA, string("manga")),
		 make_pair(ANIME, string("anime")),})
	{
		data_store.hget("user_lists", name + ":" + type.second,
			[&](cpp_redis::reply& reply){
				if(reply.is_null())
					return;
				for(const auto& score:parse_scores(reply.as_string()))
					if(!only_rated || score.second != 0){
						media_id id = {type.first, score.first};
						scores.scores[id] = score.second;
					}
			});
	}
	data_store.sync_commit();
	return scores;
}

vector<string> all_usernames(redis& data_store){
	set<string> usernames;
	data_store.hkeys("user_lists", [&](cpp_redis::reply& reply){
		auto names = reply.as_array();
		for(const auto& key:names){
			auto name = key.as_string();
			usernames.insert(name.substr(0, name.size()-6));
		}
	});
	data_store.sync_commit();
	return vector<string>(usernames.begin(), usernames.end());
}

vector<string> all_usernames(media_type type, redis& data_store){
	vector<string> usernames;
	data_store.hkeys("user_lists", [&](cpp_redis::reply& reply){
		auto names = reply.as_array();
		for(const auto& key:names){
			auto name = key.as_string();
			if(type == MANGA && name.substr(name.size()-5) == "manga"
				|| type == ANIME && name.substr(name.size()-5)=="anime"){
				usernames.push_back(name.substr(0, name.size()-6));
			}
		}
	});
	data_store.sync_commit();
	return usernames;
}