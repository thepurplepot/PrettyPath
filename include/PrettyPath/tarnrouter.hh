#include "graph.hh"
#include "pathfinder.hh"

namespace TarnRouter {
    std::vector<const TarnData> filter_tarns(const std::vector<const TarnData>& tarns, double min_elevation);
    std::pair<double, std::vector<const Node*>> find_path_between_tarns(const Graph& graph, const TarnData& tarn1, const TarnData& tarn2);
    using table_t = std::vector<std::vector<double>>;
    double tsp(const int mask, const int pos, const int n, const table_t& dist, table_t& dp);
    std::pair<std::vector<const TarnData>, std::vector<const Node*>> find_shortest_path_between_tarns(const Graph& graph, const std::vector<const TarnData>& tarns);
}
