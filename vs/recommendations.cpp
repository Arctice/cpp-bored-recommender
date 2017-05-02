#include "recommendations.h"
#include <cmath>

double pearson_correlation
(const user_scores& active, string user, redis& data_store)
{
	auto neighbour = get_scores(user, data_store);
	map<pair<media_type, int>, int> act, ngh;

	double mean_act = 0, mean_ngh = 0;

	for(auto const& score:neighbour.manga){
		ngh[make_pair(MANGA, score.first)] = score.second;
		mean_ngh += score.second;
	}
	for(auto const& score:active.manga){
		if(score.second==0) continue;
		act[make_pair(MANGA, score.first)] = score.second;
		mean_act += score.second;
	}

	for(auto const& score:neighbour.anime){
		ngh[make_pair(ANIME, score.first)] = score.second;
		mean_ngh += score.second;
	}
	for(auto const& score:active.anime){
		if(score.second==0) continue;
		act[make_pair(ANIME, score.first)] = score.second;
		mean_act += score.second;
	}

	mean_act /= act.size();
	mean_ngh /= ngh.size();

	double numerator = 0, den_act = 0, den_ngh = 0;
	int count = 0;
	for(const auto val:ngh){
		if(!act.count(val.first))
			continue;
		count++;
		double co_act = act[val.first]-mean_act;
		double co_ngh = val.second-mean_ngh;
		numerator += co_act*co_ngh;
		den_act += co_act*co_act;
		den_ngh += co_ngh*co_ngh;
	}
	if(!count){
		return 0;
	}
	return numerator/sqrt(den_act*den_ngh);
}

media_values media_score_values
(user_scores source, media_type type, redis& data_store)
{
	map<int, double> media_weights;
	map<int, int> rating_counts;

	for(const auto& user:all_usernames(type, data_store)){

		double user_weight = pearson_correlation(source, user, data_store);
		auto scores = get_scores(user, data_store);

		if(!user_weight||std::isnan(user_weight))
			continue;

		if(type==ANIME){
			for(const auto& score:scores.anime){
				media_weights[score.first] += score.second * user_weight;
				rating_counts[score.first] += 1;
			}
		}
		else{
			for(const auto& score:scores.manga){
				media_weights[score.first] += score.second * user_weight;
				rating_counts[score.first] += 1;
			}
		}
	}

	media_values values;

	for(auto& score:media_weights){
		double rating = score.second/rating_counts[score.first];
		if(rating_counts[score.first]>40)
			values.push_back(make_pair(score.first, rating));
	}

	return values;
}

media_values type_recs
(user_scores source, media_type type, redis& data_store)
{
	auto media_weights = media_score_values(source, type, data_store);

	std::sort(media_weights.begin(), media_weights.end(),
		[](const auto& a, const auto& b) { return a.second > b.second; });

	vector<pair<int, double>> recommendations;

	for(const auto& media:media_weights){
		if(type==ANIME && source.anime.count(media.first))
			continue;
		if(type==MANGA && source.manga.count(media.first))
			continue;
		recommendations.push_back(media);
	}

	return recommendations;
}

pair<media_values, media_values> recommendations
	(const string& name, redis& data_store)
{
	auto lists = get_scores("rainlife", data_store, false);

	auto recs_anime = type_recs(lists, ANIME, data_store);
	auto recs_manga = type_recs(lists, MANGA, data_store);
	return make_pair(recs_anime, recs_manga);
}