#include <iostream>
#include <queue>
#include <vector>
#include <unordered_map>
#include <map>
#include <cmath>
#include <limits>
#include <fstream>
#include <chrono>
#include <sstream>
#include <getopt.h>

double deg2rad(double deg) {
    return deg * M_PI / 180;
}

double haversine_distance(double lat1, double lon1, double lat2, double lon2) {
    double R = 6371; // Radius of the earth in km
    double dLat = deg2rad(lat2 - lat1);
    double dLon = deg2rad(lon2 - lon1); 
    double a = sin(dLat / 2) * sin(dLat / 2) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c * 1000; // Distance in meters
}

struct Node {
    std::string id_string;
    size_t id; // Hash of id_string
    double latitude, longitude;
    float elevation;

    Node(std::string id_string, double latitude, double longitude, float elevation = 0) : id_string(id_string), latitude(latitude), longitude(longitude), elevation(elevation){
        id = std::hash<std::string>{}(id_string);
    }

    Node(size_t id, double latitude, double longitude, float elevation = 0) : id(id), latitude(latitude), longitude(longitude), elevation(elevation){
        id_string = std::to_string(id);
    }

    bool operator==(const Node& other) const {
        return id == other.id;
    }

    double distance_to(double lat, double lon) const {
        return haversine_distance(latitude, longitude, lat, lon);
    }
};

struct Edge {
    long osm_id, source_id, target_id;
    double length, slope;
    int cars, difficulty;
    std::vector<long> edge_nodes;

    Edge(double length, double slope, int cars = 0, long osm_id = 0, std::vector<long> edge_nodes = {}) : length(length), slope(slope), cars(cars), osm_id(osm_id), edge_nodes(edge_nodes) {}

    double cost() {
        //TODO: add difficulty
        return length * /*(3.0f + slope/5.f) **/ (cars * 10 + 1);
    }

    double elevation_change() {
        return length * slope;
    }
};

// Store the graph as an adjacency list
// The key is the node and the value is a vector of connecting nodes (node, edge)
using Graph = std::unordered_map<Node*, std::vector<std::pair<Node*, Edge>>>;

void add_edge(Graph& graph, Node* node1, Node* node2, double length, double slope, int cars = 0, long osm_id = 0, std::vector<long> edge_nodes = {}) {
    Edge edge(length, slope, cars, osm_id, edge_nodes);
    if(graph.find(node1) == graph.end()) {
        graph[node1] = std::vector<std::pair<Node*, Edge>>();
    }
    if(graph.find(node2) == graph.end()) {
        graph[node2] = std::vector<std::pair<Node*, Edge>>();
    }
    graph[node1].push_back(std::make_pair(node2, edge));
    graph[node2].push_back(std::make_pair(node1, edge));
}


std::vector<Node*> reconstruct_path(const std::unordered_map<Node*, Node*>& came_from, Node* current) {
    std::vector<Node*> path;
    while (came_from.find(current) != came_from.end()) {
        path.push_back(current);
        current = came_from.at(current);
    }
    path.push_back(current);
    std::reverse(path.begin(), path.end());
    return path;
}

std::vector<Node*> a_star(const Graph& graph, Node* start, const Node* goal) {
    auto start_time = std::chrono::high_resolution_clock::now();
    // Priority queue of nodes to visit, sorted by the lowest f_score
    std::priority_queue<std::pair<double, Node*>, std::vector<std::pair<double, Node*>>, std::greater<std::pair<double, Node*>>> open_set;
    open_set.push(std::make_pair(0, start));

    std::unordered_map<Node*, Node*> came_from;
    std::unordered_map<Node*, double> g_score; // Cost from start to node
    for (const auto& pair : graph) {
        Node* node = pair.first;
        g_score[node] = std::numeric_limits<double>::infinity();
    }
    g_score[start] = 0;

    std::unordered_map<Node*, double> f_score; // Cost from start to goal through node
    for (const auto& pair : graph) {
        Node* node = pair.first;
        f_score[node] = std::numeric_limits<double>::infinity();
    }
    f_score[start] = start->distance_to(goal->latitude, goal->longitude);

    while (!open_set.empty()) {
        Node* current = open_set.top().second; // Get the node in open_set having the lowest f_score
        open_set.pop(); // Remove the node from open_set

        if (current == goal) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
            std::cout << "Duration: " << duration/1000.f << " ms" << std::endl;
            return reconstruct_path(came_from, current);
        }

        const auto neighbors = graph.at(current);

        for (const auto& pair : neighbors) {
            Node* neighbor = pair.first;
            Edge edge = pair.second;

            const double tentative_g_score = g_score[current] + edge.cost();
            if (tentative_g_score < g_score[neighbor]) {
                came_from[neighbor] = current;
                g_score[neighbor] = tentative_g_score;
                f_score[neighbor] = g_score[neighbor] + neighbor->distance_to(goal->latitude, goal->longitude);
                open_set.push(std::make_pair(f_score[neighbor], neighbor));
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
    std::cout << "Duration: " << duration/1000.f << " ms " << std::endl;
    // TODO: Shift start node if no path is found and try again
    return std::vector<Node*>();
}

void print_path(const std::vector<Node*>& path) {
    double min_lat = std::numeric_limits<double>::max();
    double max_lat = -std::numeric_limits<double>::max();
    double min_lon = std::numeric_limits<double>::max();
    double max_lon = -std::numeric_limits<double>::max();
    for (Node* node : path) {
        if(node->latitude < min_lat) min_lat = node->latitude;
        if(node->latitude > max_lat) max_lat = node->latitude;
        if(node->longitude < min_lon) min_lon = node->longitude;
        if(node->longitude > max_lon) max_lon = node->longitude;
        std::cout << "id: " << node->id_string << ", pos: (" << node->latitude << "," << node->longitude << "), elevation: " << node->elevation << std::endl;
    }
    std::cout << "Latitude range: " << min_lat << " - " << max_lat << std::endl;
    std::cout << "Longitude range: " << min_lon << " - " << max_lon << std::endl;
}

using MapData = std::unordered_map<long, Node*>; // Map node id to node

void write_path_to_py(const MapData& map_data, const Graph& graph, const std::vector<Node*>& path, const std::string& filename) {
    std::ofstream file(filename);
    file << "id,lat,lon,length,elevation\n";
    file << std::fixed << std::setprecision(6);

    for (int i = 1; i < path.size(); i++) {
        Node* node = path[i - 1];
        Node* next_node = path[i];

        // Find the edge between node and next_node
        for (const auto& edge_pair : graph.at(node)) {
            if (edge_pair.first == next_node) {
                Edge edge = edge_pair.second;
                // Check if the edge is in the correct direction
                if (node->id != edge.edge_nodes.front()) {
                    std::reverse(edge.edge_nodes.begin(), edge.edge_nodes.end());
                }
                // Check the end node of the edge is the same as the next node
                if (next_node->id != edge.edge_nodes.back()) {
                    std::cerr << "Error: Next node is not the end node of the edge" << std::endl;
                    continue;
                }
                if (next_node != path.back()) { // If not the last node in the path
                    edge.edge_nodes.pop_back(); // Remove the end node from the edge (Prevent duplicate nodes in the path)
                }
                const long osm_id = edge.osm_id;
                for(auto it = edge.edge_nodes.begin(); it != edge.edge_nodes.end(); it++) {
                    float length = 0;
                    const Node* edge_node = map_data.at(*it);
                    if (it + 1 != edge.edge_nodes.end()) {
                        const Node* next_edge_node = map_data.at(*(it+1));
                        length = edge_node->distance_to(next_edge_node->latitude, next_edge_node->longitude);
                    } else {
                        length = edge_node->distance_to(next_node->latitude, next_node->longitude);
                    }

                    file << osm_id << "," << edge_node->latitude << "," << edge_node->longitude << "," << length << "," << edge_node->elevation << "\n";
                }
                break;
            }
        }
    }
    file.close();
}

std::vector<long> parse_geometry(const std::string& geometry) {
    std::vector<long> edge_nodes;

    std::stringstream ss(geometry);
    std::string node_id;
    while (std::getline(ss, node_id, ',')) {
        edge_nodes.push_back(std::stol(node_id));
    }

    return edge_nodes;
}

MapData read_map_data(Graph& graph, const std::string& nodes_filename, const std::string& edges_filename) {
    MapData map_data; // Map node id to node
    double min_lat = std::numeric_limits<double>::max();
    double max_lat = -std::numeric_limits<double>::max();
    double min_lon = std::numeric_limits<double>::max();
    double max_lon = -std::numeric_limits<double>::max();

    std::cout << "Reading map data\n";

    std::ifstream nodes_file(nodes_filename);
    if (!nodes_file.is_open()) {
        std::cerr << "Error: Could not open file " << nodes_filename << std::endl;
        return map_data;
    }

    std::string line;
    std::getline(nodes_file, line); // Skip the header
    while (std::getline(nodes_file, line)) {
        std::stringstream ss(line);
        std::string field;

        // Get the node id
        std::getline(ss, field, ',');
        long id = std::stol(field);

        // Get the latitude
        std::getline(ss, field, ',');
        double latitude = std::stod(field);
        if(latitude < min_lat) min_lat = latitude;
        if(latitude > max_lat) max_lat = latitude;

        // Get the longitude
        std::getline(ss, field, ',');
        double longitude = std::stod(field);
        if(longitude < min_lon) min_lon = longitude;
        if(longitude > max_lon) max_lon = longitude;

        // Get the elevation
        std::getline(ss, field);
        float elevation = std::stof(field);

        Node* node = new Node(id, latitude, longitude, elevation);
        map_data.insert(std::make_pair(id, node));
        //graph[node] = std::vector<std::pair<Node*, Edge>>();
    }

    nodes_file.close();

    std::ifstream edges_file(edges_filename);
    if (!edges_file.is_open()) {
        std::cerr << "Error: Could not open file " << edges_filename << std::endl;
        return map_data;
    }

    std::getline(edges_file, line); // Skip the header
    while (std::getline(edges_file, line)) {
        std::stringstream ss(line);
        std::string field;

        // Get the edge id
        std::getline(ss, field, ',');
        long id = std::stol(field);

        // Get the osm_id
        std::getline(ss, field, ',');
        long osm_id = std::stol(field);

        // Get the source node id
        std::getline(ss, field, ',');
        long source_node_id = std::stol(field);

        // Get the target node id
        std::getline(ss, field, ',');
        long target_node_id = std::stol(field);

        // Get the length
        std::getline(ss, field, ',');
        double length = std::stod(field);

        // Get the slope
        std::getline(ss, field, ',');
        double slope = std::stod(field);

        // Get the difficulty
        std::getline(ss, field, ',');
        int difficulty = std::stoi(field);
        if (difficulty == -1) difficulty = 0;

        // Get the car
        std::getline(ss, field, ',');
        int car = std::stoi(field);
        if (car == -1) car = 0;

        // Get geometry of the edge
        std::getline(ss, field);
        auto edge_nodes = parse_geometry(field);

        if(map_data.find(source_node_id) == map_data.end() || map_data.find(target_node_id) == map_data.end()) {
            std::cerr << "Edge node not found in map data" << std::endl;
            continue;
        }
        Node* start_node = map_data[source_node_id];
        Node* target_node = map_data[target_node_id];
        add_edge(graph, start_node, target_node, length, slope, car, osm_id, edge_nodes);
    }

    edges_file.close();

    std::cout << "Done reading map data\n";
    std::cout << "Latitude range: " << min_lat << " -> " << max_lat << std::endl;
    std::cout << "Longitude range: " << min_lon << " -> " << max_lon << std::endl;
    return map_data;
}

std::pair<Node*, double> find_closest_node(const Graph& graph, double latitude, double longitude) {
    Node* closest_node = nullptr;
    double min_distance = std::numeric_limits<double>::max();

    for (auto& pair : graph) {
        Node* node = pair.first;
        const double distance = node->distance_to(latitude, longitude);
        if (distance < min_distance) {
            min_distance = distance;
            closest_node = node;
        }
    }

    return std::make_pair(closest_node, min_distance);
}

void print_graph_info(const Graph& graph) {
    size_t num_nodes = graph.size();
    size_t num_edges = 0;
    std::map<long, long> num_edges_per_node;
    for (const auto& pair : graph) {
        num_edges += pair.second.size();
        Node* node = pair.first;
        if (num_edges_per_node.find(pair.second.size()) == num_edges_per_node.end()) {
            num_edges_per_node[pair.second.size()] = 0;
        }
        num_edges_per_node[pair.second.size()] += 1;
    }
    std::cout << "Graph info: " << num_nodes << " nodes, " << num_edges << " edges" << std::endl;
    for (const auto& pair : num_edges_per_node) {
        std::cout << pair.second << " nodes have " << pair.first << " edges" << std::endl;
    }
}

double get_path_length(const std::vector<Node*>& path) {
    double length = 0;
    for (int i = 1; i < path.size(); i++) {
        length += path[i-1]->distance_to(path[i]->latitude, path[i]->longitude);
    }
    return length;
}

int main(int argc, char** argv) {
    std::string nodes_filename = "data/nodes.csv";
    std::string edges_filename = "data/edges.csv";
    std::string path_filename = "data/path.csv";
    double start_lat = 54.4582, start_lon = -3.0245, goal_lat = 54.4237, goal_lon = -3.0034;
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
                return 1;
        }
    }
    Graph graph;
    MapData map = read_map_data(graph, nodes_filename, edges_filename);
    //print_graph_info(graph);
    auto start = find_closest_node(graph, start_lat, start_lon);
    std::cout << "Start node: " << start.first->id_string << " at (" << start.first->latitude << "," << start.first->longitude << ") error = " << start.second << " m" << std::endl;
    auto goal = find_closest_node(graph, goal_lat, goal_lon);
    std::cout << "Goal node: " << goal.first->id_string << " at (" << goal.first->latitude << "," << goal.first->longitude << ") error = " << goal.second << " m" << std::endl;
    auto path = a_star(graph, start.first, goal.first);
    if (path.empty()) {
        std::cout << "No path found" << std::endl;
    } else {
        std::cout << "Path length: " << path.size() << " nodes, " << get_path_length(path)/1000.f << " km" << std::endl;
        //print_path(path);
        write_path_to_py(map, graph, path, path_filename);
    }
}