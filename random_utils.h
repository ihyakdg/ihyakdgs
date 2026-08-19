// random_utils.h
#pragma once
#include <random>

inline int retRandomValues(int min, int max) {
    thread_local static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

inline float retRandomValuesFloat(float min, float max) {
    thread_local static std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}
