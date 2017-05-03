#pragma once
#include "user_data.h"
#include <algorithm>

using media_values = vector<pair<media_id, double>>;

map<media_type, media_values> recommendations
	(const string& name, redis& data_store);