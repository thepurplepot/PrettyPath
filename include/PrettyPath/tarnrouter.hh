#include "graph.hh"
#include "pathfinder.hh"

namespace TarnRouter {
    std::vector<const TarnData> filter_tarns(const std::vector<const TarnData>& tarns, const double min_elevation, const long min_area);
    std::pair<double, std::vector<const Node*>> find_path_between_tarns(const Graph& graph, const TarnData& tarn1, const TarnData& tarn2);
    double tsp(const int mask, const int pos, const int n, const std::vector<double>& dist, std::vector<double>& dp);
    std::pair<std::vector<std::pair<const TarnData, size_t>>, std::vector<const Node*>> find_shortest_path_between_tarns(const Graph& graph, const std::vector<const TarnData>& tarns);
}
