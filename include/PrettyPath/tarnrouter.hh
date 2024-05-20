#include "graph.hh"
#include "pathfinder.hh"

namespace TarnRouter {
std::vector<TarnData> filter_tarns(
    const std::vector<TarnData>& tarns, const double min_elevation,
    const double max_elevation, const long min_area, const double min_latitude,
    const double max_latitude, const double min_longitude,
    const double max_longitude, const std::vector<std::string>& blacklist);
std::pair<double, std::vector<const Node*>> find_path_between_tarns(
    const Graph& graph, TarnData& tarn1, TarnData& tarn2);
double tsp(const int mask, const int pos, const int n,
           const double min_dist_per_day, const std::vector<double>& dist,
           std::unordered_map<int, double>& dp);
std::pair<std::vector<double>,
          std::unordered_map<int, std::vector<const Node*>>>
find_distances_between_tarns(const Graph& graph, std::vector<TarnData>& tarns);
void print_table(const std::vector<double>& table,
                 const std::vector<std::string>& names);
std::pair<std::vector<std::pair<const TarnData, size_t>>,
          std::vector<const Node*>>
reconstruct_path(const std::vector<TarnData>& tarns,
                 std::unordered_map<int, std::vector<const Node*>>& paths,
                 int n, const std::unordered_map<int, double>& dp);
std::pair<std::vector<std::pair<const TarnData, size_t>>,
          std::vector<const Node*>>
find_shortest_path_between_tarns(
    const Graph& graph, std::vector<TarnData>& tarn,
    const double min_dist_per_day,
    const std::pair<double, double>& start_location = {0, 0});
}  // namespace TarnRouter
