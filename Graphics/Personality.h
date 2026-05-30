#pragma once
#include <cstdlib>

struct Personality
{
	double aggression;
	double riskAversion;
	int ammoThreshold;
	int healthThreshold;

	void randomize()
	{
		aggression = 0.2 + (rand() % 60) / 100.0;
		riskAversion = 0.2 + (rand() % 60) / 100.0;
		ammoThreshold = 3 + rand() % 8;
		healthThreshold = 20 + rand() % 30;
	}
};
