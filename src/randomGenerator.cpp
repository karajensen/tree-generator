////////////////////////////////////////////////////////////////////////////////////////
// Kara Jensen - mail@karajensen.com - randomGenerator.cpp
////////////////////////////////////////////////////////////////////////////////////////

#include "randomGenerator.h"
#include "common.h"

#include <time.h>

std::default_random_engine Random::sm_generator;

void Random::Initialise()
{
    RandomizeSeed();
}

void Random::RandomizeSeed()
{
    const auto seed = static_cast<unsigned long>(time(0));
    sm_generator.seed(seed);
}

int Random::Generate(int min, int max)
{
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(sm_generator);
}

float Random::Generate(float min, float max)
{
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(sm_generator);
}

double Random::Generate(double min, double max)
{
    std::uniform_real_distribution<double> distribution(min, max);
    return distribution(sm_generator);
}