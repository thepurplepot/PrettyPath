#include "graph.hh"
#include "pathfinder.hh"
#include "tarnrouter.hh"
#include <iomanip>

namespace TarnRouter {
std::vector<const TarnData> filter_tarns(const std::vector<const TarnData>& tarns, const double min_elevation, const double max_elevation, const long min_area, const double min_latitude, const double max_latitude, const double min_longitude, const double max_longitude) {
    std::vector<const TarnData> filtered_tarns;

    std::copy_if(tarns.begin(), tarns.end(), std::back_inserter(filtered_tarns),
                 [min_elevation, max_elevation, min_area, min_latitude, max_latitude, min_longitude, max_longitude](const TarnData& tarn) {
                        if (tarn.latitude < min_latitude || tarn.latitude > max_latitude || tarn.longitude < min_longitude || tarn.longitude > max_longitude) {
                            return false;
                        }
                        if (tarn.elevation < min_elevation || tarn.elevation > max_elevation) {
                            return false;
                        }
                        if (tarn.area < min_area) {
                            return false;
                        }
                        return true;
                 });

    return filtered_tarns;
}

std::pair<double, std::vector<const Node*>> find_path_between_tarns(const Graph& graph, const TarnData& tarn1, const TarnData& tarn2) {
    auto start = graph.find_closest_node(tarn1.latitude, tarn1.longitude);
    auto goal = graph.find_closest_node(tarn2.latitude, tarn2.longitude);
    auto path = Pathfinder::a_star(graph, start.first, goal.first);
    auto length = Pathfinder::get_path_length(path);
    return std::make_pair(length, path);
}


double tsp(const int mask, const int pos, const int n, const std::vector<double>& dist, std::map<int,double>& dp) {
    if (mask == (1 << n) - 1) {
        return dist[pos*n]; // All tarns visited
    }
    if (dp.find(mask*n + pos) != dp.end()) {
        return dp.at(mask*n + pos); // Already visited
    }
    double ans = std::numeric_limits<double>::max();
    for (int i = 0; i < n; i++) {
        if ((mask & (1 << i)) == 0) {
            ans = std::min(ans, dist[pos*n + i] + tsp(mask | (1 << i), i, n, dist, dp));
        }
    }
    return dp[mask*n + pos] = ans;
}

std::pair<std::vector<double>, std::unordered_map<int, std::vector<const Node*>>> find_distances_between_tarns(const Graph& graph, const std::vector<const TarnData>& tarns) {
    const size_t n = tarns.size();
    std::vector<double> dist;
    dist.assign(n*n, 0);
    std::unordered_map<int, std::vector<const Node*>> paths;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) { // Only need to calculate distance between each pair once
            std::cout << "Calculating distance between " << tarns[i].name << " and " << tarns[j].name << std::endl;
            auto path = find_path_between_tarns(graph, tarns[i], tarns[j]);
            dist[i*n + j] = path.first;
            dist[j*n + i] = path.first;
            paths[n*i + j] = path.second;
            paths[n*j + i] = path.second;
        }
    }

    return std::make_pair(dist, paths);
}

void print_table(const std::vector<double>& table, const std::vector<std::string>& names) {
    const int n = std::sqrt(table.size());
    if (names.size() != n) {
        std::cerr << "Error: Names vetor is the wrong length for the table" << std::endl;
        return;
    }

    std::cout << std::setw(15) << " ";
    for (const auto& name : names) {
        std::cout << std::setw(15) << name;
    }
    std::cout << std::endl;

    for (size_t row = 0; row < n; row++) {
        std::cout << std::setw(15) << names[row];
        for (size_t col = 0; col < n; col++) {
            std::cout << std::setw(15) << table[row*n + col];
        }
        std::cout << std::endl;
    }
}

std::pair<std::vector<std::pair<const TarnData, size_t>>, std::vector<const Node*>> reconstruct_path(const std::vector<const TarnData>& tarns, std::unordered_map<int, std::vector<const Node*>>& paths, int n, const std::map<int,double>& dp) {
    std::vector<std::pair<const TarnData, size_t>> path;
    std::vector<const Node*> path_nodes;
    int mask = 1;
    int pos = 0;
    for (int i = 0; i < n - 1; i++) {
        int next_pos = -1;
        for (int j = 0; j < n; j++) {
            if ((mask & (1 << j)) == 0) {
                if (next_pos == -1 || dp.at((mask | (1 << j))*n + j) < dp.at((mask | (1 << next_pos))*n + next_pos)) {
                    next_pos = j;
                }
            }
        }
        path.push_back(std::make_pair(tarns[pos], paths.at(n*pos + next_pos).size()));
        std::cout << "Adding path from " << tarns[pos].name << " to " << tarns[next_pos].name << " to path" << std::endl;
        if (next_pos < pos) {
            std::cout << "Reversing path" << std::endl;
            std::reverse(paths[n*pos + next_pos].begin(), paths[n*pos + next_pos].end());
        }
        path_nodes.insert(path_nodes.end(), paths.at(n*pos + next_pos).begin(), paths.at(n*pos + next_pos).end());

        mask |= (1 << next_pos);
        pos = next_pos;
    }
    path.push_back(std::make_pair(tarns[pos], 0));
    return std::make_pair(path, path_nodes);
}

std::pair<std::vector<std::pair<const TarnData, size_t>>, std::vector<const Node*>> find_shortest_path_between_tarns(const Graph& graph, const std::vector<const TarnData>& tarns) {
    const size_t n = tarns.size();
    auto paths_table = find_distances_between_tarns(graph, tarns);
    std::vector<double> dist = paths_table.first;
    std::cout << "Tarn distance table: " << std::endl;
    std::vector<std::string> names;
    for (const auto& tarn : tarns) {
        names.push_back(tarn.name);
    }
    print_table(dist, names);
    std::unordered_map<int, std::vector<const Node*>> paths = paths_table.second;

    std::map<int, double> dp;

    tsp(1, 0, n, dist, dp);

    // Reconstruct path
    auto path = reconstruct_path(tarns, paths, n, dp);

    return path;
}
} // namespace TarnRouter