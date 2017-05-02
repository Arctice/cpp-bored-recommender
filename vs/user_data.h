#pragma once
#include <utility>
#include <string>
#include <map>
#include <iostream>
#include "data_store.h"

enum media_type {ANIME, MANGA};

struct user_scores{
	string name;
	map<int, int> anime;
	map<int, int> manga;
};

vector<pair<int, int>> parse_scores(const string&);
user_scores get_scores(const string& name, redis& data_store, bool only_rated=true);
vector<string> all_usernames(media_type type, redis& data_store);