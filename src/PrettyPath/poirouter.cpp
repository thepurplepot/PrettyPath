#include "poirouter.hh"
#include <future>
#include <iomanip>
#include <mutex>
#include "graph.hh"
#include "parser.hh"
#include "pathfinder.hh"

namespace TarnRouter {
int tsp(int mask, int pos, const std::vector<double>& dist,
        std::vector<double>& dp, std::vector<int>& parent, int n) {
  if (mask == (1 << n) - 1) {
    return dist[n * pos];  // return to starting city
  }

  if (dp[mask * n + pos] != -1) {
    return dp[mask * n + pos];
  }

  double ans = std::numeric_limits<double>::max();
  int best_next_pos = -1;

  for (int i = 0; i < n; ++i) {
    if ((mask & (1 << i)) == 0) {
      double result =
          dist[pos * n + i] + tsp(mask | (1 << i), i, dist, dp, parent, n);
      if (result < ans) {
        ans = result;
        best_next_pos = i;
      }
    }
  }

  dp[mask * n + pos] = ans;
  parent[mask * n + pos] = best_next_pos;  // store the next city in the path
  return ans;
}

std::vector<int> get_path_from_tsp(const std::vector<int>& parent, int n) {
  std::vector<int> path;
  int mask = 1, pos = 0;

  while (true) {
    path.push_back(pos);
    pos = parent[mask * n + pos];
    if (pos == -1) break;
    mask |= (1 << pos);
  }
  path.push_back(0);  // return to starting city
  return path;
}

std::vector<int> route_unordered_tarns_exact(const std::vector<double>& dist,
                                             int n) {
  std::vector<double> dp((1 << n) * n, -1);
  std::vector<int> parent((1 << n) * n, -1);
  int mask = 1;
  int pos = 0;
  double ans = tsp(mask, pos, dist, dp, parent, n);
  std::cout << "TSP distance: " << ans << std::endl;
  std::vector<int> path = get_path_from_tsp(parent, n);
  return path;
}

std::vector<POIData> filter_tarns(
    const std::vector<POIData>& tarns, const double min_elevation,
    const double max_elevation, const double min_area, const double max_area,
    const double min_latitude, const double max_latitude,
    const double min_longitude, const double max_longitude,
    const std::vector<std::string>& blacklist) {
  std::vector<POIData> filtered_tarns;
  std::cout << "Max Longitude: " << max_longitude << " Min Longitude: "
            << min_longitude << " Max Latitude: " << max_latitude
            << " Min Latitude: " << min_latitude << std::endl;

  std::copy_if(
      tarns.begin(), tarns.end(), std::back_inserter(filtered_tarns),
      [min_elevation, max_elevation, min_area, max_area, min_latitude,
       max_latitude, min_longitude, max_longitude,
       blacklist](const POIData& tarn) {
        if (tarn.latitude < min_latitude || tarn.latitude > max_latitude ||
            tarn.longitude < min_longitude || tarn.longitude > max_longitude) {
          return false;
        }
        if (tarn.elevation < min_elevation || tarn.elevation > max_elevation) {
          return false;
        }
        if (tarn.area < min_area || tarn.area > max_area) {
          return false;
        }
        for (const auto& name : blacklist) {
          if (tarn.name == name) {
            return false;
          }
        }
        return true;
      });

  return filtered_tarns;
}

std::pair<double, std::vector<const Node*>> find_path_between_tarns(
    const Graph& graph, POIData& tarn1, POIData& tarn2) {
  const Node *start, *goal;
  if (tarn1.best_node != nullptr && tarn2.best_node != nullptr) {
    start = tarn1.best_node;
    goal = tarn2.best_node;
  } else {
    start = graph.find_closest_node(tarn1.latitude, tarn1.longitude).first;
    goal = graph.find_closest_node(tarn2.latitude, tarn2.longitude).first;
  }
  auto path = Pathfinder::a_star(graph, start, goal);
  tarn1.best_node = start;
  tarn2.best_node = goal;
  auto length = Pathfinder::get_path_length(path);
  return std::make_pair(length, path);
}

double calculate_total_distance(const std::vector<int>& path,
                                const std::vector<double>& dist, const int n,
                                const double min_dist, const double max_dist,
                                std::vector<int>& fpath) {
  double total_distance = 0;
  std::vector<int> inaccessible_tarns;
  for (int i = 0; i < n; i++) {
    int from = path[i];
    int to = path[(i + 1) % n];
    double distance = dist[from * n + to];
    // while (distance > max_dist && to != 0) {
    //   inaccessible_tarns.push_back(to);
    //   i++;  // Skip the inaccessible tarn
    //   to = path[(i + 1) % n];
    //   distance = dist[from * n + to];
    // }

    if (distance < min_dist) {
      distance = distance * 10;  // Penalize short distances
    }
    total_distance += distance;
  }

  // Remove inaccessible tarns from the path
  fpath = path;
  // fpath.erase(std::remove_if(fpath.begin(), fpath.end(),
  //                            [&inaccessible_tarns](int tarn) {
  //                              return std::find(inaccessible_tarns.begin(),
  //                                               inaccessible_tarns.end(),
  //                                               tarn) !=
  //                                     inaccessible_tarns.end();
  //                            }),
  //             fpath.end());
  return total_distance / fpath.size();  // Penalize inaccessible tarns FIXME
}

// Use simulated annealing to find a good route
std::vector<int> route_unordered_tarns(const std::vector<double>& dist,
                                       const int n, const double min_dist,
                                       const double max_dist) {
  long epoch = 0;
  std::vector<int> current_path(n);
  for (int i = 0; i < n; i++) {
    current_path[i] = i;
  }

  std::vector<int> fpath;
  double current_distance = calculate_total_distance(current_path, dist, n,
                                                     min_dist, max_dist, fpath);
  std::vector<int> best_path = fpath;
  double best_distance = current_distance;

  double temperature = 100000;
  double cooling_rate = 0.99995;

  srand(time(0));

  while (temperature > 1) {
    std::vector<int> new_path = current_path;

    // Swap two random tarns
    int tarn1 = 1 + rand() % (n - 1);
    int tarn2;
    do {
      tarn2 = 1 + rand() % (n - 1);
    } while (tarn1 == tarn2);
    std::swap(new_path[tarn1], new_path[tarn2]);

    double new_distance =
        calculate_total_distance(new_path, dist, n, min_dist, max_dist, fpath);

    if (new_distance < current_distance ||
        exp((current_distance - new_distance) / temperature) >
            (rand() % 100) / 100.0) {
      current_distance = new_distance;
      current_path = new_path;
      if (current_distance < best_distance) {
        best_path = fpath;
        best_distance = current_distance;
      }
    }

    temperature *= cooling_rate;
    epoch++;
  }

  std::cout << "Epochs: " << epoch << std::endl;
  return best_path;
}

std::pair<std::vector<double>,
          std::unordered_map<int, std::vector<const Node*>>>
find_distances_between_tarns(const Graph& graph, std::vector<POIData>& tarns) {
  const size_t n = tarns.size();
  std::vector<double> dist;
  dist.assign(n * n, 0);
  std::unordered_map<int, std::vector<const Node*>> paths;

  const size_t total = n * (n - 1) / 2;
  std::mutex mux;
  size_t done = 0;

  // Progress bar lambda
  auto find_path_between_tarns_wrapper = [&mux, &done, &total, &graph, &tarns](
                                             size_t i, size_t j) {
    auto result = find_path_between_tarns(graph, tarns[i], tarns[j]);
    {
      std::lock_guard<std::mutex> lock(mux);
      done++;
      const unsigned int bar_width = 50;
      const unsigned int progress = (done * 100) / total;
      std::cout << "\rProgress: [";
      const unsigned int pos = bar_width * progress / 100;
      for (int p = 0; p < 50; p++) {
        if (p < pos)
          std::cout << "=";
        else if (p == pos)
          std::cout << ">";
        else
          std::cout << " ";
      }
      std::cout << "] " << progress << " %\r";
      std::cout.flush();
      if (result.first == 0) {
        std::cerr << "Error: No path found between tarns: " << tarns[i].name
                  << ":" << i << " and " << tarns[j].name << ":" << j
                  << std::endl;
        result.first = std::numeric_limits<double>::max();
      }
    }
    return std::make_tuple(result, i, j);
  };

  std::vector<std::future<
      std::tuple<std::pair<double, std::vector<const Node*>>, size_t, size_t>>>
      futures;

  // Compute distances from first tarn to all other tarns, set the best nodes
  // for each tarn
  for (size_t j = 1; j < n; j++) {
    futures.push_back(
        std::async(std::launch::async, find_path_between_tarns_wrapper, 0, j));
  }
  // Wait for the futures to finish and set the distances and paths
  for (auto& future : futures) {
    auto result = future.get();
    auto path = std::get<0>(result);
    size_t i = std::get<1>(result);
    size_t j = std::get<2>(result);
    dist[i * n + j] = path.first;
    dist[j * n + i] = path.first;
    paths[n * i + j] = path.second;
    paths[n * j + i] = path.second;
  }
  futures.clear();
  // Compute remaning distance pairs
  for (size_t i = 1; i < n; i++) {
    for (size_t j = i + 1; j < n;
         j++) {  // Only need to calculate distance between each pair once
      futures.push_back(std::async(std::launch::async,
                                   find_path_between_tarns_wrapper, i, j));
    }
  }
  for (auto& future : futures) {
    auto result = future.get();
    auto path = std::get<0>(result);
    size_t i = std::get<1>(result);
    size_t j = std::get<2>(result);
    dist[i * n + j] = path.first;
    dist[j * n + i] = path.first;
    paths[n * i + j] = path.second;
    paths[n * j + i] = path.second;
  }

  std::cout << std::endl;

  return std::make_pair(dist, paths);
}

void print_table(const std::vector<double>& table,
                 const std::vector<std::string>& names) {
  const int n = std::sqrt(table.size());
  if (names.size() != n) {
    std::cerr << "Error: Names vector is the wrong length for the table"
              << std::endl;
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
      std::cout << std::setw(15) << table[row * n + col];
    }
    std::cout << std::endl;
  }
}

std::vector<int> fliter_tarns_on_max_dist(std::vector<double>& dist, size_t& n,
                                          const double max_dist) {
  std::vector<int> removed_tarns_index;
  const int threshold = 2;
  for (int i = 1; i < n; i++) {
    int under_max_dist = 0;
    for (int j = 0; j < n; j++) {
      if (i == j) {
        continue;
      }
      if (dist[i * n + j] < max_dist) {
        under_max_dist++;
      }
      if (under_max_dist > threshold) {
        break;
      }
    }
    if (under_max_dist <= threshold) {
      removed_tarns_index.push_back(i);
      for (int j = 0; j < n; j++) {
        dist[i * n + j] = std::numeric_limits<double>::max();
        dist[j * n + i] = std::numeric_limits<double>::max();
      }
    }
  }
  n -= removed_tarns_index.size();
  dist.erase(std::remove_if(dist.begin(), dist.end(),
                            [max_dist](double distance) {
                              return distance ==
                                     std::numeric_limits<double>::max();
                            }),
             dist.end());
  return removed_tarns_index;
}

void renormalise_index_list(std::vector<int>& index_list,
                            std::vector<int>& removed_tarns_index) {
  std::sort(removed_tarns_index.begin(), removed_tarns_index.end(),
            std::less<int>());
  for (int removed_index : removed_tarns_index) {
    for (int& index : index_list) {
      if (index >= removed_index) {
        index++;
      }
    }
  }
}

std::pair<std::vector<std::pair<const POIData, size_t>>,
          std::vector<const Node*>>
reconstruct_path(const std::vector<POIData>& tarns,
                 std::unordered_map<int, std::vector<const Node*>>& paths,
                 const int n, const std::vector<int>& index_path) {
  std::vector<std::pair<const POIData, size_t>> path;
  std::vector<const Node*> path_nodes;

  for (int i = 0; i < index_path.size(); i++) {
    const int from = index_path[i];
    const int to = index_path[(i + 1) % index_path.size()];
    path.push_back(std::make_pair(tarns[from], paths.at(n * from + to).size()));
    if (to < from) {
      std::reverse(paths[n * from + to].begin(), paths[n * from + to].end());
    }
    path_nodes.insert(path_nodes.end(), paths.at(n * from + to).begin(),
                      paths.at(n * from + to).end());
  }
  // Add the last tarn
  path.push_back(std::make_pair(tarns[0], 0));
  return std::make_pair(path, path_nodes);
}

std::pair<std::vector<std::pair<const POIData, size_t>>,
          std::vector<const Node*>>
find_shortest_path_between_tarns(
    const Graph& graph, std::vector<POIData>& tarns, const double min_dist,
    const double max_dist, const std::pair<double, double>& start_location) {
  if (start_location.first != 0 && start_location.second != 0) {
    tarns.insert(tarns.begin(), POIData("Start", start_location.first,
                                        start_location.second, 0, 0, 0));
  }
  size_t n = tarns.size();
  auto paths_table = find_distances_between_tarns(graph, tarns);
  std::vector<double> dist = paths_table.first;
  auto removed_tarns_index = fliter_tarns_on_max_dist(dist, n, max_dist);
  if (removed_tarns_index.size() > 0) {
    std::cout << "Removed tarns: ";
    for (int index : removed_tarns_index) {
      std::cout << tarns[index].name << ", ";
    }
    std::cout << std::endl;
  }
  std::unordered_map<int, std::vector<const Node*>> paths = paths_table.second;

  auto index_path = route_unordered_tarns(dist, n, min_dist, max_dist);
  // auto tsp_path = route_unordered_tarns_exact(dist, n);
  renormalise_index_list(index_path, removed_tarns_index);
  // renormalise_index_list(tsp_path, removed_tarns_index);
  std::cout << "Index path: ";
  for (int i = 0; i < index_path.size(); i++) {
    std::cout << index_path[i] << " ";
  }
  std::cout << std::endl;
  // std::cout << "TSP path: ";
  // for (int i = 0; i < tsp_path.size(); i++) {
  //   std::cout << tsp_path[i] << " ";
  // }
  // std::cout << std::endl;

  auto path = reconstruct_path(tarns, paths, tarns.size(), index_path);
  // auto path = reconstruct_path(tarns, paths, n, tsp_path);

  return path;
}

std::pair<std::vector<std::pair<const POIData, size_t>>,
          std::vector<const Node*>>
find_shortest_path_between_ordered_tarns(
    const Graph& graph, std::vector<POIData>& tarn,
    const std::pair<double, double>& start_location) {
  std::pair<std::vector<std::pair<const POIData, size_t>>,
            std::vector<const Node*>>
      result;
  if (start_location.first != 0 && start_location.second != 0) {
    tarn.insert(tarn.begin(), POIData("Start", start_location.first,
                                      start_location.second, 0, 0, 0));
  }

  const size_t total = tarn.size() - 1;
  size_t done = 0;

  for (size_t i = 1; i <= tarn.size(); i++) {
    auto path =
        find_path_between_tarns(graph, tarn[i - 1], tarn[i % tarn.size()]);
    if (path.first == 0) {
      std::cerr << "Error: No path found between tarns: " << tarn[i - 1].name
                << " and " << tarn[i].name << std::endl;
      continue;
    }
    done++;
    result.first.push_back(std::make_pair(tarn[i - 1], path.second.size()));
    result.second.insert(result.second.end(), path.second.begin(),
                         path.second.end());
    const unsigned int bar_width = 50;
    const unsigned int progress = (done * 100) / total;
    std::cout << "\rProgress: [";
    const unsigned int pos = bar_width * progress / 100;
    for (int p = 0; p < 50; p++) {
      if (p < pos)
        std::cout << "=";
      else if (p == pos)
        std::cout << ">";
      else
        std::cout << " ";
    }
    std::cout << "] " << progress << " %\r";
    std::cout.flush();
  }
  std::cout << std::endl;

  result.first.push_back(std::make_pair(tarn.back(), 0));
  return result;
}
}  // namespace TarnRouter