#include "recommendations.h"
#include <cmath>

map<media_id, double> normalize_scores
(const map<media_id, int>& scores)
{
	double mean = 0;
	for(auto const& score:scores)
		mean += score.second;
	mean /= scores.size();

	map<media_id, double> normalized;
	for(const auto& score:scores)
		normalized[score.first] = score.second-mean;
	return normalized;
}

double pearson_correlation
(const map<media_id, double>& act, const map<media_id, double>& ngh)
{
	double numerator = 0, den_act = 0, den_ngh = 0;
	int count = 0;
	for(const auto val:ngh){
		if(!act.count(val.first)) continue;
		count++;
		numerator += act.at(val.first)*val.second;
		den_act += act.at(val.first)*act.at(val.first);
		den_ngh += val.second*val.second;
	}
	if(!count) return 0;

	auto distance = numerator/sqrt(den_act*den_ngh);
	distance *= min(count, 50.0)/50.0;

	return distance;
}

map<media_type, media_values> media_score_values
(user_scores source, redis& data_store)
{
	map<media_id, double> media_weights;
	map<media_id, int> rating_counts;

	auto active_arr = normalize_scores(
		get_scores(source.name, data_store).scores);

	for(const auto& user:all_usernames(data_store)){

		auto scores = get_scores(user, data_store).scores;
		double user_weight = pearson_correlation(
			active_arr, normalize_scores(scores));
		
		if(!user_weight || std::isnan(user_weight))
			continue;
		
		for(const auto& score : scores){
			media_weights[score.first] += score.second * user_weight;
			rating_counts[score.first] += 1;
		}
	}

	map<media_type, media_values> values;

	for(auto& score : media_weights){
		auto type = score.first.first;
		auto mal_id = score.first.second;
		double rating = score.second/rating_counts[score.first];

		if(rating_counts[score.first]>40)
			values[type].push_back({score.first, rating});
	}

	return values;
}

map<media_type, media_values> recommendations
(const string& name, redis& data_store)
{
	auto source = get_scores(name, data_store, false);
	auto media_weights = media_score_values(source, data_store);

	for(const auto& type:{ANIME, MANGA}){
		std::sort(media_weights[type].begin(), media_weights[type].end(),
			[](const auto& a, const auto& b) { return a.second > b.second; });
	}

	for(const auto& type:{ANIME, MANGA}){
		remove_if(media_weights[type].begin(), media_weights[type].end(),
			[&](auto elem){return source.scores.count(elem.first); });
	}

	return media_weights;
}
