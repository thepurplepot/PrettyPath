#include <queue>
#include <stack>
#include <unordered_set>
#include "graph.hh"

namespace Pathfinder {

using open_set_t = std::priority_queue<std::pair<double, const Node*>, std::vector<std::pair<double, const Node*>>, std::greater<std::pair<double, const Node*>>>;

std::pair<double, const Node*> find_nearby_node(const std::vector<const Node*> attempted_goals, const double variation, const Graph& graph);
std::pair<double, const Node*> find_nearby_connected_node(const Node* const desired, const Node* connected_node, const double variation, const Graph& graph);
bool is_connected(const Graph& graph, const Node* start, const Node* goal);
bool visit_next_node(const Graph& graph, std::stack<const Node*>& stack, std::unordered_set<const Node*>& visited_from_this_side, std::unordered_set<const Node*>& visited_from_other_side);
std::vector<const Node*> reconstruct_path(const std::unordered_map<const Node*, const Node*>& came_from, const Node* current);
void init(const Graph& graph, const Node* start, const Node* goal, std::unordered_map<const Node*, const Node*>& came_from, std::unordered_map<const Node*, double>& g_score, std::unordered_map<const Node*, double>& f_score, open_set_t& open_set);
std::vector<const Node*> a_star(const Graph& graph, const Node*& start, const Node*& goal);
bool find_connected_start_and_goal(const Graph& graph, const Node*& start, const Node*& goal);
void print_path(const std::vector<Node*>& path);
double get_path_length(const std::vector<const Node*>& path);

} // namespace Pathfinder