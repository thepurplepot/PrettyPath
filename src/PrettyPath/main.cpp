#include <getopt.h>
#include "config.hh"
#include "graph.hh"
#include "parser.hh"
#include "pathfinder.hh"
#include "tarnrouter.hh"

void handle_option(int argc, char** argv, std::string& config_filename) {
  int opt;
  char config_filename_c[50];
  while ((opt = getopt(argc, argv, "s:g:")) != -1) {
    switch (opt) {
      case 'c':
        sscanf(optarg, "%s", config_filename_c);
        config_filename = std::string(config_filename_c);
        break;
      default:
        std::cerr << "Usage: " << argv[0] << " -c <config_file>" << std::endl;
        exit(1);
    }
  }
}

Config::config_t Config::c;

int main(int argc, char** argv) {
  std::string config_filename = "config.json";
  handle_option(argc, argv, config_filename);

  Config::get_config(config_filename);

  Parser parser(Config::c.nodes_filename, Config::c.edges_filename);
  Graph graph;
  MapData map = parser.read_map_data(graph);
  auto tarns = parser.read_tarn_data(Config::c.tarns_filename);
  auto filtered_tarns = TarnRouter::filter_tarns(
      tarns, Config::c.minimum_tarn_elevation, Config::c.maximum_tarn_elevation,
      Config::c.minimum_tarn_area, Config::c.minimum_latitude,
      Config::c.maximum_latitude, Config::c.minimum_longitude,
      Config::c.maximum_latitude, Config::c.completed_tarns);
  std::cout << "Tarns must have an elevation between "
            << Config::c.minimum_tarn_elevation << " and "
            << Config::c.maximum_tarn_elevation << " m" << std::endl;
  std::cout << "Tarns must have an area of at least "
            << Config::c.minimum_tarn_area << " m^2" << std::endl;
  std::cout << "Filtered tarns:" << std::endl;
  for (auto tarn : filtered_tarns) {
    std::cout << "\"" << tarn.name << "\""
              << " Elevation: " << tarn.elevation << " m Area: " << tarn.area
              << " m^2" << std::endl;
  }
  std::cout << std::endl;
  auto path = TarnRouter::find_shortest_path_between_tarns(
      graph, filtered_tarns, Config::c.min_dist_per_day,
      Config::c.start_location);
  auto tarn_path = path.first;
  if (path.first.empty()) {
    std::cout << "No path found" << std::endl;
  } else {
    std::cout << "Total path length: "
              << Pathfinder::get_path_length(path.second) << " m" << std::endl;
    std::cout << "Tarn order:" << std::endl;
    for (auto pair : tarn_path) {
      auto tarn = pair.first;
      std::cout << tarn.name << " at (" << tarn.latitude << ", "
                << tarn.longitude << ")" << std::endl;
    }
    parser.write_tarn_paths(map, graph, path, Config::c.path_dir,
                            Config::c.gpx_filename);
  }
  parser.clean_map_data(map);
}