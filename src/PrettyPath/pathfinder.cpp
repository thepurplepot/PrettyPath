#include <queue>
#include "graph.hh"
#include "pathfinder.hh"

namespace Pathfinder {

const Node* find_nearby_node(const std::vector<const Node*> attempted_goals, const double variation, const Graph& graph) {
    const Node* nearby_node = nullptr;
    double min_distance = std::numeric_limits<double>::max();
    const Node* prev_goal = attempted_goals.back();
    const auto prev_goal_location = prev_goal->get_location();

    graph.apply_to_nodes([&](const Node* node) {
        bool is_attempted = false;
        for (const Node* goal : attempted_goals) {
            if (node == goal) {
                is_attempted = true;
                break;
            }
        }
        if (is_attempted) {
            return;
        }
        double distance = node->distance_to(prev_goal_location.first, prev_goal_location.second);

        if (distance < min_distance && distance >= variation) {
            nearby_node = node;
            min_distance = distance;
        }
    });

    return nearby_node;
}

std::vector<const Node*> reconstruct_path(const std::unordered_map<const Node*, const Node*>& came_from, const Node* current) {
    std::vector<const Node*> path;
    while (came_from.find(current) != came_from.end()) {
        path.push_back(current);
        current = came_from.at(current);
    }
    path.push_back(current);
    std::reverse(path.begin(), path.end());
    return path;
}

void init(const Graph& graph, const Node* start, const Node* goal, std::unordered_map<const Node*, const Node*>& came_from, std::unordered_map<const Node*, double>& g_score, std::unordered_map<const Node*, double>& f_score, open_set_t& open_set, long& searched_nodes) {
    searched_nodes = 0;
    open_set.push(std::make_pair(0, start));

    came_from.clear();
    graph.apply_to_nodes([&](const Node* node) {
        g_score[node] = std::numeric_limits<double>::infinity();
    });
    g_score[start] = 0;

    graph.apply_to_nodes([&](const Node* node) {
        f_score[node] = std::numeric_limits<double>::infinity();
    });
    f_score[start] = start->distance_to(goal->get_location().first, goal->get_location().second);
}

using open_set_t = std::priority_queue<std::pair<double, const Node*>, std::vector<std::pair<double, const Node*>>, std::greater<std::pair<double, const Node*>>>;

std::vector<const Node*> a_star(const Graph& graph, const Node* start, const Node* goal) {
    auto start_time = std::chrono::high_resolution_clock::now();
    const Node* current_goal = goal;
    std::vector<const Node*> attempted_goals = {goal}; // List of goal nodes attempted
    // Priority queue of nodes to visit, sorted by the lowest f_score
    open_set_t open_set;

    std::unordered_map<const Node*, const Node*> came_from; // Map of nodes to their parent nodes
    std::unordered_map<const Node*, double> g_score; // Cost from start to node
    std::unordered_map<const Node*, double> f_score; // Cost from start to goal through node
    long searched_nodes;
    init(graph, start, current_goal, came_from, g_score, f_score, open_set, searched_nodes);

    while(true) {
        if(open_set.empty()) {
            if (attempted_goals.size()  > 50) {
                std::cout << "Error: Tried 100 different goal nodes, no path found! Returned best attempt." << std::endl;
                double min_distance = std::numeric_limits<double>::max();
                const Node* best_attempted_goal = nullptr;
                for(const auto node_pair : came_from) {
                    const Node* node = node_pair.first;
                    const Node* parent = node_pair.second;
                    double distance = node->distance_to(current_goal->get_location().first, current_goal->get_location().second);
                    if(distance < min_distance) {
                        min_distance = distance;
                        best_attempted_goal = node;
                    }
                }
                return reconstruct_path(came_from, best_attempted_goal);
            }
            std::cout << "No path found after searching " << searched_nodes << " nodes, trying again with nearby goal\n";
            const Node* nearby_goal = find_nearby_node(attempted_goals, 100, graph); // Find a nearby goal
            if(nearby_goal == nullptr) {
                std::cerr << "Error: No nearby node found" << std::endl;
                return {};
            }
            attempted_goals.push_back(nearby_goal);
            const auto location = nearby_goal->get_location();
            std::cout << "New goal: " << nearby_goal->get_id() << " at (" << location.first << "," << location.second << ")" << std::endl;
            current_goal = nearby_goal;
            init(graph, start, current_goal, came_from, g_score, f_score, open_set, searched_nodes);
        }

        const Node* current = open_set.top().second; // Get the node in open_set having the lowest f_score
        open_set.pop(); // Remove the node from open_set
        searched_nodes++;

        if (current == current_goal) {
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
            std::cout << "Duration: " << duration/1000.f << " ms" << std::endl;
            return reconstruct_path(came_from, current);
        }

        const auto neighbours = graph.get_neighbours(current);

        for (const auto& pair : neighbours) {
            const Node* neighbour = pair.first;
            const Edge edge = pair.second;

            const double tentative_g_score = g_score[current] + edge.cost();
            if (tentative_g_score < g_score[neighbour]) {
                came_from[neighbour] = current;
                g_score[neighbour] = tentative_g_score;
                f_score[neighbour] = g_score[neighbour] + neighbour->distance_to(current_goal->get_location().first, current_goal->get_location().second);
                open_set.push(std::make_pair(f_score[neighbour], neighbour));
            }
        }
    }
}

void print_path(const std::vector<const Node*>& path) {
    double min_lat = std::numeric_limits<double>::max();
    double max_lat = -std::numeric_limits<double>::max();
    double min_lon = std::numeric_limits<double>::max();
    double max_lon = -std::numeric_limits<double>::max();
    for (const Node* node : path) {
        const auto location = node->get_location();
        if(location.first < min_lat) min_lat = location.first;
        if(location.first > max_lat) max_lat = location.first;
        if(location.second < min_lon) min_lon = location.second;
        if(location.second > max_lon) max_lon = location.second;
        std::cout << "id: " << node->get_id() << ", pos: (" << location.first << "," << location.second << "), elevation: " << node->get_elevation() << std::endl;
    }
    std::cout << "Latitude range: " << min_lat << " - " << max_lat << std::endl;
    std::cout << "Longitude range: " << min_lon << " - " << max_lon << std::endl;
}

double get_path_length(const std::vector<const Node*>& path) {
    double length = 0;
    for (int i = 1; i < path.size(); i++) {
        const auto location1 = path[i - 1]->get_location();
        const auto location2 = path[i]->get_location();
        length += path[i - 1]->distance_to(location2.first, location2.second);
    }
    return length;
}
} // namespace Pathfinder