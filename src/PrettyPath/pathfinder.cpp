#include "pathfinder.hh"

namespace Pathfinder {

std::pair<double, const Node*> find_nearby_node(
    const std::vector<const Node*> attempted_goals, const double variation,
    const Graph& graph) {
  const Node* nearby_node = nullptr;
  double min_distance = std::numeric_limits<double>::max();
  const Node* first_goal = attempted_goals.front();
  const auto first_goal_location = first_goal->get_location();

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
    double distance = node->distance_to(first_goal_location.first,
                                        first_goal_location.second);

    if (distance < min_distance && distance >= variation) {
      nearby_node = node;
      min_distance = distance;
    }
  });

  return std::make_pair(min_distance, nearby_node);
}

std::pair<double, const Node*> find_nearby_connected_node(
    const Node* const desired, const Node* connected_node,
    const double variation, const Graph& graph) {
  const auto desired_location = desired->get_location();
  const Node* nearby_node = nullptr;
  double min_distance = std::numeric_limits<double>::max();
  std::vector<const Node*> attempted_nodes;

  for (double radius = variation; radius < 10 * variation;
       radius += variation) {
    graph.apply_to_nodes([&](const Node* node) {
      if (std::find(attempted_nodes.begin(), attempted_nodes.end(), node) !=
          attempted_nodes.end()) {
        return;
      }
      double distance =
          node->distance_to(desired_location.first, desired_location.second);
      if (distance < radius && distance < min_distance) {
        attempted_nodes.push_back(node);
        if (is_connected(graph, node, connected_node)) {
          nearby_node = node;
          min_distance = distance;
        }
      }
    });
    if (nearby_node != nullptr) {
      break;
    }
  }

  return std::make_pair(min_distance, nearby_node);
}

std::vector<const Node*> reconstruct_path(
    const std::unordered_map<const Node*, const Node*>& came_from,
    const Node* current) {
  std::vector<const Node*> path;
  while (came_from.find(current) != came_from.end()) {
    path.push_back(current);
    current = came_from.at(current);
  }
  path.push_back(current);
  std::reverse(path.begin(), path.end());
  return path;
}

void init(const Graph& graph, const Node* start, const Node* goal,
          std::unordered_map<const Node*, const Node*>& came_from,
          std::unordered_map<const Node*, double>& g_score,
          std::unordered_map<const Node*, double>& f_score,
          open_set_t& open_set, long& searched_nodes) {
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
  f_score[start] = start->distance_to(goal->get_location().first,
                                      goal->get_location().second);
}

bool is_connected(const Graph& graph, const Node* start, const Node* goal) {
  // Bidirectional DFS
  std::unordered_set<const Node*> visited_from_start;
  std::unordered_set<const Node*> visited_from_goal;
  std::stack<const Node*> stack_from_start;
  std::stack<const Node*> stack_from_goal;
  stack_from_start.push(start);
  stack_from_goal.push(goal);
  while (!stack_from_start.empty() && !stack_from_goal.empty()) {
    if (visit_next_node(graph, stack_from_start, visited_from_start,
                        visited_from_goal)) {
      return true;
    }
    if (visit_next_node(graph, stack_from_goal, visited_from_goal,
                        visited_from_start)) {
      return true;
    }
  }
  return false;
}

bool visit_next_node(const Graph& graph, std::stack<const Node*>& stack,
                     std::unordered_set<const Node*>& visited_from_this_side,
                     std::unordered_set<const Node*>& visited_from_other_side) {
  const Node* node = stack.top();
  stack.pop();
  if (visited_from_other_side.count(node)) {
    return true;
  }
  if (!visited_from_this_side.count(node)) {
    visited_from_this_side.insert(node);
    for (const auto neighbour : graph.get_neighbours(node)) {
      auto neighbour_node = neighbour.first;
      stack.push(neighbour_node);
    }
  }
  return false;
}

bool find_connected_start_and_goal(const Graph& graph, const Node*& start,
                                   const Node*& goal) {
  size_t attempts = 0;
  double variation = 50;
  const Node* new_start = nullptr;
  std::vector<const Node*> attempted_goals = {goal};
  double goal_error = 0;

  while (new_start == nullptr && attempts < 15) {
    if (attempts % 5 == 0 && attempts != 0) {
      variation *= 2;
    }
    auto start_node = find_nearby_connected_node(start, goal, variation, graph);
    new_start = start_node.second;
    if (new_start != nullptr) {
      start = new_start;
      const double error = start_node.first + goal_error;
      // std::cout << "Total distance error: " << error << " after " << attempts
      // << " attempts" << std::endl;
      return true;
    }
    auto goal_node = find_nearby_node(attempted_goals, variation, graph);
    goal = goal_node.second;
    if (goal == nullptr) {
      std::cerr << "Error: No nearby goal node found!" << std::endl;
      return false;
    }
    goal_error = goal_node.first;
    attempted_goals.push_back(goal);
    attempts++;
  }

  std::cerr << "Error: No connection between start and goal found!"
            << std::endl;
  return false;
}

std::vector<const Node*> a_star(const Graph& graph, const Node*& start,
                                const Node*& goal) {
  if (!is_connected(graph, start, goal)) {
    // std::cout << "Start and goal are not connected" << std::endl;
    if (!find_connected_start_and_goal(graph, start, goal)) {
      std::cout << "No connected start and goal found" << std::endl;
      return {};
    }
  }
  // Priority queue of nodes to visit, sorted by the lowest f_score
  open_set_t open_set;

  std::unordered_map<const Node*, const Node*>
      came_from;  // Map of nodes to their parent nodes
  std::unordered_map<const Node*, double> g_score;  // Cost from start to node
  std::unordered_map<const Node*, double>
      f_score;  // Cost from start to goal through node
  long searched_nodes;
  init(graph, start, goal, came_from, g_score, f_score, open_set,
       searched_nodes);

  while (true) {
    if (open_set.empty()) {
      std::cerr << "Error: No path found after searching " << searched_nodes
                << " nodes" << std::endl;
      return {};
    }

    const Node* current =
        open_set.top()
            .second;  // Get the node in open_set having the lowest f_score
    open_set.pop();   // Remove the node from open_set
    searched_nodes++;

    if (current == goal) {
      return reconstruct_path(came_from, current);
    }

    const auto neighbours = graph.get_neighbours(current);

    for (auto& pair : neighbours) {
      const Node* neighbour = pair.first;
      const Edge& edge = pair.second;

      const double tentative_g_score = g_score[current] + edge.cost();
      if (tentative_g_score < g_score[neighbour]) {
        came_from[neighbour] = current;
        g_score[neighbour] = tentative_g_score;
        f_score[neighbour] =
            g_score[neighbour] +
            neighbour->distance_to(goal->get_location().first,
                                   goal->get_location().second);
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
    if (location.first < min_lat) min_lat = location.first;
    if (location.first > max_lat) max_lat = location.first;
    if (location.second < min_lon) min_lon = location.second;
    if (location.second > max_lon) max_lon = location.second;
    std::cout << "id: " << node->get_id() << ", pos: (" << location.first << ","
              << location.second << "), elevation: " << node->get_elevation()
              << std::endl;
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
}  // namespace Pathfinder
