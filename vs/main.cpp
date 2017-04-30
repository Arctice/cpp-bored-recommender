#include "data_store.h"
#include "user_data.h"
#include <cmath>

double pearson_correlation
	(const user_scores& active, string user, redis& data_store)
{
	auto neighbour = get_scores(user, data_store);
	map<pair<media_type, int>, int> act, ngh;
	
	double mean_act = 0, mean_ngh = 0;

	for(auto const& score:neighbour.manga){
		if(score.second==0) continue;
		ngh[make_pair(MANGA, score.first)] = score.second;
		mean_ngh += score.second;
	}
	for(auto const& score:active.manga){
		if(score.second==0) continue;
		act[make_pair(MANGA, score.first)] = score.second;
		mean_act += score.second;
	}

	for(auto const& score:neighbour.anime){
		if(score.second==0) continue;
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

vector<pair<int, double>> media_score_values
	(user_scores source, media_type type, redis& data_store)
{
	map<int, double> media_values;
	map<int, int> rating_counts;
	
	for(const auto& user:all_usernames(type, data_store)){

		double user_weight = pearson_correlation(source, user, data_store);
		auto scores = get_scores(user, data_store);

		if(std::isnan(user_weight)) continue;

		if(type==ANIME){
			for(const auto& score:scores.anime){
				if(!score.second) continue;

				if(!media_values.count(score.first)){
					media_values[score.first] = 0;
					rating_counts[score.first] = 0;
				}
				media_values[score.first] += score.second * user_weight;
				rating_counts[score.first] += 1;
			}
		}
		else{
			for(const auto& score:scores.manga){
				if(!score.second) continue;

				if(!media_values.count(score.first)){
					media_values[score.first] = 0;
					rating_counts[score.first] = 0;
				}
				media_values[score.first] += score.second * user_weight;
				rating_counts[score.first] += 1;
			}
		}
	}

	vector<pair<int, double>> values;

	for(auto& score:media_values){
		double rating = score.second / rating_counts[score.first];
		if(rating_counts[score.first]>40)
			values.push_back(make_pair(score.first, rating));
	}

	return values;
}

auto recommendations
	(user_scores source, media_type type,
	 int cutoff, redis& data_store)
{
	auto r = media_score_values(source, type, data_store);
	std::sort(r.begin(), r.end(), [](auto a, auto b){
		return a.second>b.second;
	});
	return r;
}

auto recs (const string& name, redis& data_store){
	auto lists = get_scores("rainlife", data_store);
	user_scores taste, ignore;

	for(const auto& score:lists.anime){
		if(score.second==0)
			ignore.anime[score.first] = score.second;
		else
			taste.anime[score.first] = score.second;
	}
	for(const auto& score:lists.manga){
		if(score.second==0)
			ignore.manga[score.first] = score.second;
		else
			taste.manga[score.first] = score.second;
	}

	auto recs_anime = recommendations(taste, ANIME, 30, data_store);
	auto recs_manga = recommendations(taste, MANGA, 30, data_store);
	return make_pair(recs_anime, recs_manga);
}

auto recs() {return std::vector<std::pair<int, double>>(); }

int main(){
	auto r = redis_connect();

	auto z = recs("rainlife", *r);

	cout<<"anime\n";
	for(int i = 0; i<10; i++){
		cout<<z.first[i].first<<": "<<z.first[i].second<<endl;
	}
	cout<<"\nmanga\n";
	for(int i = 0; i<10; i++){
		cout<<z.second[i].first<<": "<<z.second[i].second<<endl;
	}
	auto f = pearson_correlation(get_scores("rainlife", *r), "bildoktorn", *r);
	//cout<<f;

	std::cin.get();
}