#include "graph.hh"
#include "parser.hh"
#include "pathfinder.hh"
#include "tarnrouter.hh"
#include <getopt.h>


void handle_option(int argc, char** argv, std::string& nodes_filename, std::string& edges_filename, std::string& path_dir, double& start_lat, double& start_lon, double& goal_lat, double& goal_lon) {
    int opt;
    while((opt = getopt(argc, argv, "s:g:")) != -1) {
        switch(opt) {
            case 's':
                sscanf(optarg, "%lf,%lf", &start_lat, &start_lon);
                std::cout << "Start: " << start_lat << " " << start_lon << std::endl;
                break;
            case 'g':
                sscanf(optarg, "%lf,%lf", &goal_lat, &goal_lon);
                std::cout << "Goal: " << goal_lat << " " << goal_lon << std::endl;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -s <start_lat,start_lon> -g <goal_lat,goal_lon>" << std::endl;
                exit(1);
        }
    }
}

int main(int argc, char** argv) {
    std::string nodes_filename = "data/nodes.csv";
    std::string edges_filename = "data/edges.csv";
    std::string tarns_filename = "data/tarns.csv";
    std::string path_dir = "data/path/";
    double start_lat = 54.4582, start_lon = -3.0245, goal_lat = 54.4237, goal_lon = -3.0034;
    handle_option(argc, argv, nodes_filename, edges_filename, path_dir, start_lat, start_lon, goal_lat, goal_lon);
    
    Parser parser(nodes_filename, edges_filename);
    Graph graph;
    MapData map = parser.read_map_data(graph);
    auto tarns = parser.read_tarn_data(tarns_filename);
    auto filtered_tarns = TarnRouter::filter_tarns(tarns, MINIMUM_TARN_ELEVATION, MAXIMUM_TARN_ELEVATION, MINIMUM_TARN_AREA, MINIMUM_LATITUDE, MAXIMUM_LATITUDE, MINIMUM_LONGITUDE, MAXIMUM_LONGITUDE);
    std::cout << "Tarns must have an elevation between " << MINIMUM_TARN_ELEVATION << " and " << MAXIMUM_TARN_ELEVATION << " m" << std::endl;
    std::cout << "Tarns must have an area of at least " << MINIMUM_TARN_AREA << " m^2" << std::endl;
    std::cout << "Filtered tarns:" << std::endl;
    for (auto tarn : filtered_tarns) {
        std::cout << "\"" << tarn.name << "\"" << " Elevation: " << tarn.elevation << " m Area: " << tarn.area << " m^2" << std::endl;
    }
    std::cout << std::endl;
    auto path = TarnRouter::find_shortest_path_between_tarns(graph, filtered_tarns);
    auto tarn_path = path.first;
    if (path.first.empty()) {
        std::cout << "No path found" << std::endl;
    } else {
        std::cout << "Total path length: " << Pathfinder::get_path_length(path.second) << " m" << std::endl;
        std::cout << "Tarn order:" << std::endl;
        for(auto pair : tarn_path) {
            auto tarn = pair.first;
            std::cout << tarn.name << " at (" << tarn.latitude << ", " << tarn.longitude << ")" << std::endl;
        }
        parser.write_tarn_paths(map, graph, path, "data/path/");
    }
    parser.clean_map_data(map);
}