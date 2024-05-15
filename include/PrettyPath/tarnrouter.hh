#include "graph.hh"
#include "pathfinder.hh"

namespace TarnRouter {
    std::vector<const TarnData> filter_tarns(const std::vector<const TarnData>& tarns, const double min_elevation, const double max_elevation, const long min_area, const double min_latitude, const double max_latitude, const double min_longitude, const double max_longitude);
    std::pair<double, std::vector<const Node*>> find_path_between_tarns(const Graph& graph, const TarnData& tarn1, const TarnData& tarn2);
    double tsp(const int mask, const int pos, const int n, const std::vector<double>& dist, std::map<int,double>& dp);
    std::pair<std::vector<double>, std::unordered_map<int, std::vector<const Node*>>> find_distances_between_tarns(const Graph& graph, const std::vector<const TarnData>& tarns);
    void print_table(const std::vector<double>& table, const std::vector<std::string>& names);
    std::pair<std::vector<std::pair<const TarnData, size_t>>, std::vector<const Node*>> reconstruct_path(const std::vector<const TarnData>& tarns, std::unordered_map<int, std::vector<const Node*>>& paths, int n, const std::map<int,double>& dp);
    std::pair<std::vector<std::pair<const TarnData, size_t>>, std::vector<const Node*>> find_shortest_path_between_tarns(const Graph& graph, const std::vector<const TarnData>& tarns);
}
