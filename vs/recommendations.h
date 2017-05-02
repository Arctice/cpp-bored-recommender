#pragma once
#include "user_data.h"

using media_values = vector<pair<int, double>>;

pair<media_values, media_values> recommendations
	(const string& name, redis& data_store);