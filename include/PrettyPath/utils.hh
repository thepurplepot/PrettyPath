#include <cmath>
#pragma once

namespace utils {
    inline double deg2rad(const double deg) {
        return deg * M_PI / 180;
    }

    inline double haversine_distance(const double lat1, const double lon1, const double lat2, const double lon2) {
        const double R = 6371; // Radius of the earth in km
        const double dLat = deg2rad(lat2 - lat1);
        const double dLon = deg2rad(lon2 - lon1); 
        const double a = sin(dLat / 2) * sin(dLat / 2) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * sin(dLon / 2) * sin(dLon / 2);
        const double c = 2 * atan2(sqrt(a), sqrt(1 - a));
        return R * c * 1000; // Distance in m
    }
} // namespace utils



