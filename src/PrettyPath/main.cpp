#include "graph.hh"
#include "parser.hh"
#include "pathfinder.hh"
#include "tarnrouter.hh"
#include <getopt.h>


void handle_option(int argc, char** argv, std::string& nodes_filename, std::string& edges_filename, std::string& path_filename, double& start_lat, double& start_lon, double& goal_lat, double& goal_lon) {
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
    std::string path_filename = "data/path.csv";
    double start_lat = 54.4582, start_lon = -3.0245, goal_lat = 54.4237, goal_lon = -3.0034;
    handle_option(argc, argv, nodes_filename, edges_filename, path_filename, start_lat, start_lon, goal_lat, goal_lon);
    
    Parser parser(nodes_filename, edges_filename);
    Graph graph;
    MapData map = parser.read_map_data(graph);
    auto tarns = parser.read_tarn_data("data/tarns.csv");
    auto filtered_tarns = TarnRouter::filter_tarns(tarns, 300);
    std::cout << "Filtered tarns:" << std::endl;
    for (auto tarn : filtered_tarns) {
        std::cout << tarn.name << std::endl;
    }
    auto path = TarnRouter::find_shortest_path_between_tarns(graph, filtered_tarns);
    auto tarn_path = path.first;
    if (path.first.empty()) {
        std::cout << "No path found" << std::endl;
    } else {
        std::cout << "Path length: " << path.second.size() << std::endl;
        std::cout << "Tarn order:" << std::endl;
        for(auto tarn : tarn_path) {
            std::cout << tarn.name << std::endl;
        }
        parser.write_path_to_py(map, graph, path.second, path_filename);
    }

    // auto start = graph.find_closest_node(start_lat, start_lon);
    // std::cout << "Start node: " << start.first->get_id() << " error = " << start.second << " m" << std::endl;
    // auto goal = graph.find_closest_node(goal_lat, goal_lon);
    // std::cout << "Goal node: " << goal.first->get_id() << " error = " << goal.second << " m" << std::endl;
    // auto path = Pathfinder::a_star(graph, start.first, goal.first);
    // if (path.empty()) {
    //     std::cout << "No path found" << std::endl;
    // } else {
    //     parser.write_path_to_py(map, graph, path, path_filename);
    // }
    parser.clean_map_data(map);
}