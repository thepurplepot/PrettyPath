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
  Config::check_config();
  Config::print_config();

  Parser parser(Config::c.nodes_filename, Config::c.edges_filename);
  Graph graph;
  MapData map = parser.read_map_data(graph);
  std::pair<std::vector<std::pair<const TarnData, size_t>>,
            std::vector<const Node*>>
      path;
  if (!Config::c.use_ordered_tarns) {
    auto tarns = parser.read_tarn_data(Config::c.tarns_filename);
    auto filtered_tarns = TarnRouter::filter_tarns(
        tarns, Config::c.min_tarn_elevation, Config::c.max_tarn_elevation,
        Config::c.min_tarn_area, Config::c.max_tarn_area,
        Config::c.min_latitude, Config::c.max_latitude, Config::c.min_longitude,
        Config::c.max_latitude, Config::c.tarn_blacklist);
    std::cout << "Filtered tarns:" << std::endl;
    for (auto tarn : filtered_tarns) {
      std::cout << "\"" << tarn.name << "\""
                << " Elevation: " << tarn.elevation << " m Area: " << tarn.area
                << " m^2" << std::endl;
    }
    std::cout << std::endl;
    path = TarnRouter::find_shortest_path_between_tarns(
        graph, filtered_tarns, Config::c.min_path_length,
        Config::c.start_location);
  } else {
    auto tarns = parser.read_ordered_tarn_data(Config::c.tarns_filename);
    path = TarnRouter::find_shortest_path_between_ordered_tarns(
        graph, tarns, Config::c.start_location);
  }

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
    parser.write_tarn_paths(map, graph, path, Config::c.output_dir,
                            Config::c.gpx_filename);
  }
  parser.clean_map_data(map);
}