#include "graph.hh"
#include "pathfinder.hh"
#include "tarnrouter.hh"
#include <iomanip>

namespace TarnRouter {
std::vector<const TarnData> filter_tarns(const std::vector<const TarnData>& tarns, double min_elevation) {
    std::vector<const TarnData> filtered_tarns;

    std::copy_if(tarns.begin(), tarns.end(), std::back_inserter(filtered_tarns),
                 [min_elevation](const TarnData& tarn) { return tarn.elevation >= min_elevation; });

    return filtered_tarns;
}

std::pair<double, std::vector<const Node*>> find_path_between_tarns(const Graph& graph, const TarnData& tarn1, const TarnData& tarn2) {
    auto start = graph.find_closest_node(tarn1.latitude, tarn1.longitude);
    auto goal = graph.find_closest_node(tarn2.latitude, tarn2.longitude);
    auto path = Pathfinder::a_star(graph, start.first, goal.first);
    auto length = Pathfinder::get_path_length(path);
    return std::make_pair(length, path);
}


double tsp(const int mask, const int pos, const int n, const table_t& dist, table_t& dp) {
    if (mask == (1 << n) - 1) {
        return dist[pos][0]; // All tarns visited
    }
    if (dp[mask][pos] != -1) {
        return dp[mask][pos]; // Already visited
    }
    double ans = std::numeric_limits<double>::max();
    for (int i = 0; i < n; i++) {
        if ((mask & (1 << i)) == 0) {
            ans = std::min(ans, dist[pos][i] + tsp(mask | (1 << i), i, n, dist, dp));
        }
    }
    return dp[mask][pos] = ans;
}

std::pair<table_t, std::unordered_map<int, std::vector<const Node*>>> find_distances_between_tarns(const Graph& graph, const std::vector<const TarnData>& tarns) {
    int n = tarns.size();
    table_t dist;
    dist.assign(n, std::vector<double>(n, 0));
    std::unordered_map<int, std::vector<const Node*>> paths;

    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) { // Only need to calculate distance between each pair once
            std::cout << "Calculating distance between " << tarns[i].name << " and " << tarns[j].name << std::endl;
            auto path = find_path_between_tarns(graph, tarns[i], tarns[j]);
            dist[i][j] = path.first;
            dist[j][i] = path.first;
            paths[n*i + j] = path.second;
            paths[n*j + i] = path.second;
        }
    }

    return std::make_pair(dist, paths);
}

void print_table(const table_t& table) {
    std::array<std::string, 7> names = { "Lambfoot Dub","Beckhead Tarn","Foxes Tarn","Hard Tarn","four tarn bog"};
    std::cout << std::setw(15) << " ";
    for (const auto& name : names) {
        std::cout << std::setw(15) << name;
    }
    std::cout << std::endl;

    for (size_t i = 0; i < table.size(); i++) {
        auto row = table[i];
        std::cout << std::setw(15) << names[i];
        for (const auto& cell : row) {
            std::cout << std::setw(15) << cell;
        }
        std::cout << std::endl;
    }
}

std::pair<std::vector<const TarnData>, std::vector<const Node*>> find_shortest_path_between_tarns(const Graph& graph, const std::vector<const TarnData>& tarns) {
    int n = tarns.size();
    auto paths_table = find_distances_between_tarns(graph, tarns);
    table_t dist = paths_table.first;
    std::cout << "Tarn distance table: " << std::endl;
    print_table(dist);
    std::unordered_map<int, std::vector<const Node*>> paths = paths_table.second;

    table_t dp;
    dp.assign(1 << n, std::vector<double>(n, -1));

    double length = tsp(1, 0, n, dist, dp);

    // Reconstruct path
    std::vector<const TarnData> path;
    std::vector<const Node*> path_nodes;
    int mask = 1;
    int pos = 0;
    for (int i = 0; i < n - 1; i++) {
        path.push_back(tarns[pos]);
        // Reverse path to get correct order
        if (i > pos) {
            std::reverse(paths[n*pos + i].begin(), paths[n*pos + i].end());
        }
        path_nodes.insert(path_nodes.end(), paths[n*pos + i].begin(), paths[n*pos + i].end());
        int next_pos = -1;
        for (int j = 0; j < n; j++) {
            if ((mask & (1 << j)) == 0) {
                if (next_pos == -1 || dist[pos][j] + dp[mask | (1 << j)][j] < dist[pos][next_pos] + dp[mask | (1 << next_pos)][next_pos]) {
                    next_pos = j;
                }
            }
        }
        mask |= (1 << next_pos);
        pos = next_pos;
    }

    return std::make_pair(path, path_nodes);
}
} // namespace TarnRouter