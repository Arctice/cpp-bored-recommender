#include "user_data.h"
#include <sstream>
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

user_scores get_scores(const string& name, redis& data_store){
	user_scores scores;
	// can we write this better with hana?
	for(auto& type:
		{make_pair(ref(scores.manga), string("manga")),
		 make_pair(ref(scores.anime), string("anime")),}){
		data_store.hget("user_lists", name + ":" + type.second, [&](cpp_redis::reply& reply){
			if(!reply.is_null())
				for(const auto& score:parse_scores(reply.as_string()))
					type.first[score.first] = score.second;
		});
	}
	data_store.commit();
	data_store.sync_commit();
	return scores;
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