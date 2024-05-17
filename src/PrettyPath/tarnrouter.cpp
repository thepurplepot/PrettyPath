#include "graph.hh"
#include "pathfinder.hh"
#include "tarnrouter.hh"
#include "parser.hh"
#include <iomanip>

namespace TarnRouter {
std::vector<TarnData> filter_tarns(const std::vector<TarnData>& tarns, const double min_elevation, const double max_elevation, const long min_area, const double min_latitude, const double max_latitude, const double min_longitude, const double max_longitude) {
    std::vector<TarnData> filtered_tarns;

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

std::pair<double, std::vector<const Node*>> find_path_between_tarns(const Graph& graph, TarnData& tarn1, TarnData& tarn2) {
    const Node *start, *goal;
    if(tarn1.best_node != nullptr && tarn2.best_node != nullptr) {
        start = tarn1.best_node;
        goal = tarn2.best_node;
    } else {
        start = graph.find_closest_node(tarn1.latitude, tarn1.longitude).first;
        goal = graph.find_closest_node(tarn2.latitude, tarn2.longitude).first;
    }
    auto path = Pathfinder::a_star(graph, start, goal);
    tarn1.best_node = start;
    tarn2.best_node = goal;
    // std::cout << tarn1.name << ": (" << start->get_location().first << "," << start->get_location().second << ") " << tarn2.name << ": (" << goal->get_location().first << "," << goal->get_location().second << ")" << std::endl;
    auto length = Pathfinder::get_path_length(path);
    return std::make_pair(length, path);
}

double tsp(const int mask, const int pos, const int n, const double min_dist_per_day, const std::vector<double>& dist, std::unordered_map<int,double>& dp) {
    if (mask == (1 << n) - 1) {
        return dist[pos*n]; // All tarns visited
    }
    if (dp.find(mask*n + pos) != dp.end()) {
        return dp.at(mask*n + pos); // Already visited
    }
    bool valid_node_exists = false;
    double ans = std::numeric_limits<double>::max();
    for (int i = 0; i < n; i++) {
        if ((mask & (1 << i)) == 0) {
            if(dist[pos*n + i] < min_dist_per_day) {
                continue;
            }
            valid_node_exists = true;
            ans = std::min(ans, dist[pos*n + i] + tsp(mask | (1 << i), i, n, min_dist_per_day, dist, dp));
        }
    }
    if (!valid_node_exists) {
        return ans; // Dont visit any more tarns
    }
    return dp[mask*n + pos] = ans;
}

std::pair<std::vector<double>, std::unordered_map<int, std::vector<const Node*>>> find_distances_between_tarns(const Graph& graph, std::vector<TarnData>& tarns) {
    const size_t n = tarns.size();
    std::vector<double> dist;
    dist.assign(n*n, 0);
    std::unordered_map<int, std::vector<const Node*>> paths;

    const size_t total = n * (n-1) / 2;
    size_t done = 0;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) { // Only need to calculate distance between each pair once
            auto path = find_path_between_tarns(graph, tarns[i], tarns[j]);
            dist[i*n + j] = path.first;
            dist[j*n + i] = path.first;
            paths[n*i + j] = path.second;
            paths[n*j + i] = path.second;

            // Progress bar
            done++;
            const unsigned int bar_width = 50;
            const unsigned int progress = (done * 100) / total;
            std::cout << "\rProgress: [";
            const unsigned int pos = bar_width * progress / 100;
            for (int p = 0; p < 50; p++) {
                if (p < pos) std::cout << "=";
                else if (p == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << progress << " %\r";
            std::cout.flush();
        }
    }

    std::cout << std::endl;

    return std::make_pair(dist, paths);
}

void print_table(const std::vector<double>& table, const std::vector<std::string>& names) {
    const int n = std::sqrt(table.size());
    if (names.size() != n) {
        std::cerr << "Error: Names vetor is the wrong length for the table" << std::endl;
        return;
    }
    
    std::cout << "Tarn distance table: " << std::endl;
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

std::pair<std::vector<std::pair<const TarnData, size_t>>, std::vector<const Node*>> reconstruct_path(const std::vector<TarnData>& tarns, std::unordered_map<int, std::vector<const Node*>>& paths, int n, const std::unordered_map<int,double>& dp) {
    std::vector<std::pair<const TarnData, size_t>> path;
    std::vector<const Node*> path_nodes;
    int mask = 1;
    int pos = 0;
    for (int i = 0; i < n - 1; i++) {
        int next_pos = -1;
        for (int j = 0; j < n; j++) {
            if ((mask & (1 << j)) == 0) {
                if (next_pos == -1) {
                    next_pos = j;
                    continue;
                }
                int candidate_index = (mask | (1 << j))*n + j;
                int next_pos_index = (mask | (1 << next_pos))*n + next_pos;
                if (dp.find(candidate_index) == dp.end()) {
                    continue;
                }
                if (dp.find(next_pos_index) == dp.end()) {
                    std::cout << "Error: next_pos_index not found" << std::endl;
                    next_pos = j;
                    continue;
                }
                if (dp.at(candidate_index) < dp.at(next_pos_index)) {
                    next_pos = j;
                }
            }
        }
        path.push_back(std::make_pair(tarns[pos], paths.at(n*pos + next_pos).size()));
        // std::cout << "Adding path from " << tarns[pos].name << " to " << tarns[next_pos].name << " to path" << std::endl;
        if (next_pos < pos) {
            // std::cout << "Reversing path" << std::endl;
            std::reverse(paths[n*pos + next_pos].begin(), paths[n*pos + next_pos].end());
        }
        path_nodes.insert(path_nodes.end(), paths.at(n*pos + next_pos).begin(), paths.at(n*pos + next_pos).end());

        mask |= (1 << next_pos);
        pos = next_pos;
    }
    path.push_back(std::make_pair(tarns[pos], 0));
    return std::make_pair(path, path_nodes);
}

//TODO add a start index??
std::pair<std::vector<std::pair<const TarnData, size_t>>, std::vector<const Node*>> find_shortest_path_between_tarns(const Graph& graph, std::vector<TarnData>& tarns, const double min_dist_per_day) {
    const size_t n = tarns.size();
    auto paths_table = find_distances_between_tarns(graph, tarns);
    std::vector<double> dist = paths_table.first;
    // std::vector<std::string> names;
    // for (const auto& tarn : tarns) {
    //     names.push_back(tarn.name);
    // }
    // print_table(dist, names);
    std::unordered_map<int, std::vector<const Node*>> paths = paths_table.second;

    std::unordered_map<int, double> dp;

    tsp(1, 0, n, min_dist_per_day, dist, dp);

    auto path = reconstruct_path(tarns, paths, n, dp);

    return path;
}
} // namespace TarnRouter