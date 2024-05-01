import networkx as nx
import math
import heapq
import matplotlib.pyplot as plt

def calculate_cost(distance, steepness):
    return distance * (1 + steepness/100)

def a_star(graph: nx.Graph, start, goal):
    open_set = PriorityQueue() # Priority queue with elements (f_score, node)
    open_set.put((0, start)) # Add the start node to the open set

    came_from = {} # Dictionary to store the parent node of each node
    g_score = {node: float('inf') for node in graph.nodes} # Dictionary to store the cost of the cheapest path to each node
    g_score[start] = 0

    f_score = {node: float('inf') for node in graph.nodes} # Dictionary to store the total cost of the cheapest path to each node
    f_score[start] = heuristic(graph, start, goal)

    while not open_set.empty():
        _, current = open_set.get()

        if current == goal:
            return reconstruct_path(came_from, current)

        for neighbor in graph.neighbors(current):
            tentative_g_score = g_score[current] + calculate_cost(graph.edges[current, neighbor]['distance'], graph.edges[current, neighbor]['steepness'])

            if tentative_g_score < g_score[neighbor]:
                came_from[neighbor] = current
                g_score[neighbor] = tentative_g_score
                f_score[neighbor] = g_score[neighbor] + heuristic(graph, neighbor, goal)

                if neighbor not in open_set:
                    open_set.put((f_score[neighbor], neighbor))

    return None

def heuristic(graph: nx.Graph, node, goal):
    # Get the position attributes of the nodes
    x1, y1 = graph.nodes[node]['pos']
    x2, y2 = graph.nodes[goal]['pos']

    # Calculate the Euclidean distance between the nodes
    distance = math.sqrt((x2 - x1)**2 + (y2 - y1)**2)

    # Calculate the average steepness of the graph
    total_steepness = sum(data['steepness'] for u, v, data in graph.edges(data=True))
    average_steepness = total_steepness / graph.number_of_edges()

    # Return the cost, adjusted by the average steepness
    return calculate_cost(distance, average_steepness)

def reconstruct_path(came_from, current):
    path = [current]
    while current in came_from:
        current = came_from[current]
        path.append(current)
    path.reverse()
    return path

class PriorityQueue:
    def __init__(self):
        self.elements = []

    def empty(self):
        return len(self.elements) == 0

    def put(self, item):
        heapq.heappush(self.elements, item)

    def get(self):
        return heapq.heappop(self.elements)
    
    def __contains__(self, item):
        return item in [element[1] for element in self.elements]
    
def main():
    G = nx.Graph()

    G.add_node('A', pos=(0, 0))
    G.add_node('B', pos=(1, 2))
    G.add_node('C', pos=(2, 0))
    G.add_node('D', pos=(3, 2))
    G.add_node('E', pos=(4, 0))
    G.add_edge('A', 'B', distance=1.5, steepness=10)
    G.add_edge('A', 'C', distance=1.0, steepness=-20)
    G.add_edge('B', 'D', distance=0.5, steepness=30)
    G.add_edge('C', 'D', distance=2.0, steepness=-10)
    G.add_edge('D', 'E', distance=1.0, steepness=0)

    # Run the A* algorithm
    path = a_star(G, 'A', 'E')

    print(path)
    
    pos = nx.get_node_attributes(G, 'pos')
    nx.draw(G, pos, with_labels=True)
    path_edges = list(zip(path, path[1:]))
    nx.draw_networkx_nodes(G, pos, nodelist=path, node_color='r')
    nx.draw_networkx_edges(G, pos, edgelist=path_edges, edge_color='r', width=2)
    edge_labels = {(u, v): f"d: {data['distance']}, s: {data['steepness']}" for u, v, data in G.edges(data=True)}
    nx.draw_networkx_edge_labels(G, pos, edge_labels=edge_labels)

    plt.show()

if __name__ == '__main__':
    main()