#pragma once

#define LENGTH_WEIGHT 1.f
#define ELEVATION_WEIGHT 1/5.f
#define DIFFICULTY_WEIGHT 0.1f
#define CARS_WEIGHT 10.f

#define MAX_SLOPE 3.f

#define COST(length, slope, cars, difficulty) (LENGTH_WEIGHT * length + ELEVATION_WEIGHT * (slope + MAX_SLOPE) + CARS_WEIGHT * (cars + 1) + DIFFICULTY_WEIGHT * (difficulty + 1))
