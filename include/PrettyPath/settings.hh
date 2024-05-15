#pragma once

#define LENGTH_WEIGHT 1.f
#define ELEVATION_WEIGHT 1/5.f
#define DIFFICULTY_WEIGHT 0.1f
#define CARS_WEIGHT 10.f

#define MAX_SLOPE 3.f

// Avoid roads, steep inclines and difficult paths
#define COST(length, slope, cars, difficulty) (LENGTH_WEIGHT * length + ELEVATION_WEIGHT * (slope + MAX_SLOPE) + CARS_WEIGHT * (cars + 1) + DIFFICULTY_WEIGHT * (difficulty + 1))


#define MINIMUM_TARN_ELEVATION 500
#define MAXIMUM_TARN_ELEVATION 800
#define MINIMUM_TARN_AREA 500
#define MINIMUM_LATITUDE 54.2
#define MAXIMUM_LATITUDE 54.7
#define MINIMUM_LONGITUDE -3.5
#define MAXIMUM_LONGITUDE -2.5
