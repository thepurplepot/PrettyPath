#include "graph.hh"
#include "pathfinder.hh"

namespace TarnRouter {
std::vector<POIData> filter_tarns(
    const std::vector<POIData>& tarns, const double min_elevation,
    const double max_elevation, const double min_area, const double max_area,
    const double min_latitude, const double max_latitude,
    const double min_longitude, const double max_longitude,
    const std::vector<std::string>& blacklist);
std::pair<double, std::vector<const Node*>> find_path_between_tarns(
    const Graph& graph, POIData& tarn1, POIData& tarn2);
double calculate_total_distance(const std::vector<int>& path,
                                const std::vector<double>& dist, const int n,
                                const double min_dist, const double max_dist);
std::vector<int> route_unordered_tarns(const std::vector<double>& dist,
                                       const int n, const double min_dist,
                                       const double max_dist);
std::pair<std::vector<double>,
          std::unordered_map<int, std::vector<const Node*>>>
find_distances_between_tarns(const Graph& graph, std::vector<POIData>& tarns);
void print_table(const std::vector<double>& table,
                 const std::vector<std::string>& names);
std::pair<std::vector<std::pair<const POIData, size_t>>,
          std::vector<const Node*>>
reconstruct_path(const std::vector<POIData>& tarns,
                 std::unordered_map<int, std::vector<const Node*>>& paths,
                 const int n, const std::vector<int>& index_path);
std::pair<std::vector<std::pair<const POIData, size_t>>,
          std::vector<const Node*>>
find_shortest_path_between_tarns(
    const Graph& graph, std::vector<POIData>& tarn, const double min_dist,
    const double max_dist,
    const std::pair<double, double>& start_location = {0, 0});
std::pair<std::vector<std::pair<const POIData, size_t>>,
          std::vector<const Node*>>
find_shortest_path_between_ordered_tarns(
    const Graph& graph, std::vector<POIData>& tarn,
    const std::pair<double, double>& start_location = {0, 0});
}  // namespace TarnRouter
