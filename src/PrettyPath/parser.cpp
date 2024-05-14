#include "parser.hh"
#include <filesystem>


// Allocate memory for static variables
std::string Parser::m_nodes_filename;
std::string Parser::m_edges_filename;
double Parser::m_min_lat = std::numeric_limits<double>::max();
double Parser::m_max_lat = -std::numeric_limits<double>::max();
double Parser::m_min_lon = std::numeric_limits<double>::max();
double Parser::m_max_lon = -std::numeric_limits<double>::max();

Parser::Parser(std::string nodes_filename, std::string edges_filename) {
    m_nodes_filename = nodes_filename;
    m_edges_filename = edges_filename;
}

std::vector<node_id_t> Parser::parse_nodes(const std::string& edge_nodes_string) {
    std::vector<node_id_t> edge_nodes;

    std::stringstream ss(edge_nodes_string);
    std::string node_id;
    while (std::getline(ss, node_id, ',')) {
        edge_nodes.push_back(std::stol(node_id));
    }

    return edge_nodes;
}

MapData Parser::read_map_data(Graph& graph) {
    MapData map_data; // Map node id to node

    std::cout << "Reading map data\n";

    std::ifstream nodes_file(m_nodes_filename);
    if (!nodes_file.is_open()) {
        std::cerr << "Error: Could not open file " << m_nodes_filename << std::endl;
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
        if(latitude < m_min_lat) m_min_lat = latitude;
        if(latitude > m_max_lat) m_max_lat = latitude;

        // Get the longitude
        std::getline(ss, field, ',');
        double longitude = std::stod(field);
        if(longitude < m_min_lon) m_min_lon = longitude;
        if(longitude > m_max_lon) m_max_lon = longitude;

        // Get the elevation
        std::getline(ss, field);
        float elevation = std::stof(field);

        const Node* node = new Node(id, latitude, longitude, elevation);
        map_data.insert(std::make_pair(id, node));
    }

    nodes_file.close();

    std::ifstream edges_file(m_edges_filename);
    if (!edges_file.is_open()) {
        std::cerr << "Error: Could not open file " << m_edges_filename << std::endl;
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
        if (difficulty == -1) difficulty = 0; // Normalize difficulty

        // Get the car
        std::getline(ss, field, ',');
        int car = std::stoi(field);
        if (car == -1) car = 0; // Normalize car

        // Get geometry of the edge
        std::getline(ss, field);
        auto edge_nodes = parse_nodes(field);

        if(map_data.find(source_node_id) == map_data.end() || map_data.find(target_node_id) == map_data.end()) {
            std::cerr << "Error: Edge node not found in map data for edge: " << osm_id << std::endl;
            continue;
        }
        if(source_node_id == target_node_id) {
            std::cerr << "Error: Source and Target node of edge: " << osm_id << " are the same!" << std::endl;
        }
        const Node* start_node = map_data[source_node_id];
        const Node* target_node = map_data[target_node_id];
        graph.add_edge(start_node, target_node, length, slope, car, osm_id, edge_nodes);
    }

    edges_file.close();

    std::cout << "Done reading map data\n";
    std::cout << "Latitude range: " << m_min_lat << " -> " << m_max_lat << std::endl;
    std::cout << "Longitude range: " << m_min_lon << " -> " << m_max_lon << std::endl;
    return map_data;
}

std::vector<const TarnData> Parser::read_tarn_data(const std::string& filename) {
    std::vector<const TarnData> tarn_data;

    std::ifstream tarns_file(filename);
    if (!tarns_file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return tarn_data;
    }

    std::string line;
    std::getline(tarns_file, line); // Skip the header
    while (std::getline(tarns_file, line)) {
        std::stringstream ss(line);
        std::string field;

        // Get the osm_id
        std::getline(ss, field, ',');
        long osm_id = std::stol(field);

        // Get the name
        std::getline(ss, field, '"');
        std::getline(ss, field, '"');
        std::string name = field;
        std::getline(ss, field, ',');

        // Get the latitude
        std::getline(ss, field, ',');
        double latitude = std::stod(field);

        // Get the longitude
        std::getline(ss, field, ',');
        double longitude = std::stod(field);

        // Get the elevation
        std::getline(ss, field, ',');
        float elevation = std::stof(field);

        // Get the area
        std::getline(ss, field);
        unsigned long area = std::stoul(field);

        tarn_data.push_back(TarnData(name, latitude, longitude, osm_id, elevation, area));
    }

    tarns_file.close();

    return tarn_data;
}

void Parser::write_path_to_py(const MapData& map_data, const Graph& graph, const std::vector<const Node*>& path, const std::string& filename) {
    std::ofstream file(filename);
    long node_counter = 0;
    double total_length = 0;

    file << "id,lat,lon,length,elevation\n";
    file << std::fixed << std::setprecision(6);

    for (int i = 1; i < path.size(); i++) {
        const Node* node = path[i - 1];
        const Node* next_node = path[i];
        if(node == nullptr) {
            std::cerr << "Error: Node is null" << std::endl; 
            continue;
        }

        // Find the edge between node and next_node
        for (const auto& edge_pair : graph.get_neighbours(node)) {
            if (edge_pair.first == next_node) {
                Edge edge = edge_pair.second;
                // if (edge.get_difficulty() != 0) { //DEBUG
                //     std::cerr << "Warning: Difficulty is not 0 for " << edge.get_osm_id() << std::endl;
                // }
                // Check if the edge is in the correct direction
                edge.reverse_if_needed(node->get_id());
                auto edge_nodes = edge.get_edge_nodes();
                // Check the end node of the edge is the same as the next node
                if (next_node->get_id() != edge_nodes.back()) {
                    std::cerr << "Error: Next node is not the end node of the edge" << std::endl;
                    continue;
                }
                if (next_node != path.back()) { // If not the last node in the path
                    edge_nodes.pop_back(); // Remove the end node from the edge (Prevent duplicate nodes in the path)
                }
                const long osm_id = edge.get_osm_id();
                for(auto it = edge_nodes.begin(); it != edge_nodes.end(); it++) {
                    float length = 0;
                    const Node* edge_node = map_data.at(*it);
                    if (it + 1 != edge_nodes.end()) {
                        const Node* next_edge_node = map_data.at(*(it+1));
                        const auto location = next_edge_node->get_location();
                        length = edge_node->distance_to(location.first, location.second);
                    } else {
                        const auto location = next_node->get_location();
                        length = edge_node->distance_to(location.first, location.second);
                    }
                    const auto location = edge_node->get_location();
                    file << osm_id << "," << location.first << "," << location.second << "," << length << "," << edge_node->get_elevation() << "\n";
                    node_counter++;
                    total_length += length;
                }
                break;
            }
        }
    }
    file.close();
    std::cout << "Wrote " << node_counter << " nodes to " << filename << std::endl;
    std::cout << "Total length: " << total_length/1000.f << " km" << std::endl;
}

void Parser::write_tarn_paths(const MapData& map_data, const Graph& graph, const std::pair<std::vector<std::pair<const TarnData, size_t>>, std::vector<const Node*>>& tarns_path, const std::string& file_dir) {
    auto tarns = tarns_path.first;
    auto path = tarns_path.second;
    size_t path_start = 0;
    size_t edges_written = 0;
    for (const auto & entry : std::filesystem::directory_iterator(file_dir)) {
        std::filesystem::remove(entry);
    }
    for(size_t i = 0; i < tarns.size() - 1; i++) {
        auto path_length = tarns[i].second;
        edges_written += path_length;
        auto start_tarn = tarns[i].first;
        auto end_tarn = tarns[(i+1)%tarns.size()].first;
        std::cout << "Writing path from " << start_tarn.name << " to " << end_tarn.name << " with " << path_length << " nodes" << std::endl;
        std::string filename = file_dir + "path_" + start_tarn.name_without_spaces() + "_to_" + end_tarn.name_without_spaces() + ".csv";
        std::vector<const Node*> sub_path(path.begin() + path_start, path.begin() + path_start + path_length);
        path_start = path_length;
        write_path_to_py(map_data, graph, sub_path, filename);
    }
    std::cout << "Wrote " << edges_written << " edges to files, total in path is: " << path.size() << std::endl;
}


void Parser::clean_map_data(MapData& map_data) {
    for (auto& pair : map_data) {
        delete pair.second;
    }
    map_data.clear();
}