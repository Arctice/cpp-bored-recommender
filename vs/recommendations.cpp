#include "recommendations.h"
#include <cmath>

pair<map<media_id, double>, double> normalize_scores
(const map<media_id, int>& scores)
{
	double mean = 0;
	for(auto const& score:scores)
		mean += score.second;
	mean /= scores.size();

	map<media_id, double> normalized;
	for(const auto& score:scores)
		normalized[score.first] = score.second - mean;

	return {normalized, mean};
}

double pearson_correlation
(const map<media_id, double>& act, const map<media_id, double>& ngh)
{
	double numerator = 0, den_act = 0, den_ngh = 0;
	int shared_count = 0;
	for(const auto val:ngh){
		if(!act.count(val.first)) continue;
		shared_count++;
		
		auto co_act = act.at(val.first);
		auto co_ngh = val.second;

		numerator += co_act * co_ngh;
		den_act += co_act * co_act;
		den_ngh += co_ngh * co_ngh;
	}
	if(!shared_count) return 0;

	auto distance = numerator/sqrt(den_act*den_ngh);
	distance *= min(static_cast<double>(shared_count), 50.0)/50.0;

	return distance;
}

map<media_type, media_values> media_score_values
(user_scores source, redis& data_store)
{
	map<media_id, vector<pair<double, double>>> media_weights;
	
	auto active_user_data = normalize_scores(
		get_scores(source.name, data_store).scores);
	auto active_mean = active_user_data.second;
	auto active_vector = active_user_data.first;

	for(const auto& user:all_usernames(data_store)){
		auto scores = get_scores(user, data_store).scores;
		auto neighbour_data = normalize_scores(scores);
		auto neighbour_vector = neighbour_data.first;
		auto neighbour_mean = neighbour_data.second;

		double user_weight = pearson_correlation(
			active_vector, neighbour_vector);
		
		if(!user_weight || std::isnan(user_weight))
			continue;
		
		for(const auto& score : scores){
			auto normalized_score = score.second - neighbour_mean;
			media_weights[score.first].push_back({normalized_score, user_weight});
		}
	}

	map<media_type, media_values> values;

	for(auto& score : media_weights){
		auto type = score.first.first;
		auto mal_id = score.first.second;
		auto& scores = score.second;

		std::sort(scores.begin(), scores.end(),
			[](const auto& L, const auto& R){ return L.first>R.first; });

		auto top_scorers_count = scores.size()/10;
		auto total_weights = 0.0, total_value = 0.0;

		for(auto i = (size_t)0; i<top_scorers_count; ++i){
			auto weight = scores[i].second;
			if(weight < 0)
				continue;
			total_weights += abs(scores[i].second);
			total_value += scores[i].first*scores[i].second;
		}

		if(total_weights < 1)
			continue;
		
		total_value /= total_weights;
		if(total_value < 0)
			continue;
		
		values[type].push_back({score.first, total_value});
	}

	return values;
}

map<media_type, media_values> recommendations
(const string& name, redis& data_store)
{
	auto source = get_scores(name, data_store, false);
	auto media_weights = media_score_values(source, data_store);

	auto rated_source = get_scores(name, data_store, true);

	for(const auto& type:{ANIME, MANGA}){
		std::sort(media_weights[type].begin(), media_weights[type].end(),
			[](const auto& a, const auto& b) { return a.second > b.second; });
	}

	for(const auto& type:{ANIME, MANGA}){
		remove_if(media_weights[type].begin(), media_weights[type].end(),
			[&](auto elem){ return rated_source.scores.count(elem.first); });
	}

	return media_weights;
}
