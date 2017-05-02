#pragma once
#include <utility>
#include <string>
#include <map>
#include <iostream>
#include "data_store.h"

enum media_type {ANIME, MANGA};
using media_id = pair<media_type, int>;

struct user_scores{
	string name;
	map<media_id, int> scores;
};

vector<pair<int, int>> parse_scores(const string&);
user_scores get_scores(const string& name, redis& data_store, bool only_rated=true);
vector<string> all_usernames(media_type type, redis& data_store);
vector<string> all_usernames(redis& data_store);