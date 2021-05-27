#include "Utils.h"

std::default_random_engine generator;
float purdue::normal_random(float mean, float sig) {
	std::normal_distribution<float> distribution(mean, sig);
	return distribution(generator);
}