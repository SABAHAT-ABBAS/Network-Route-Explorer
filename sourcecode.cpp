#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <queue>
#include <climits> // For INT_MAX
#include <fstream>
#include <stack>
#include <sstream>
#include <string>
#include <conio.h> // For _kbhit() and _getch() - Windows-specific
#include <utility> // For std::move
#include <limits>  // For std::numeric_limits

using namespace std;
using namespace sf;

constexpr float PI = 3.14159265f;
constexpr int MAX_V = 100;
constexpr float ROUTER_RADIUS = 20.0f;
constexpr float GRAPH_CENTER_X = 400.0f;
constexpr float GRAPH_CENTER_Y = 300.0f;
constexpr float GRAPH_RADIUS = 200.0f;

// Represents an edge in the adjacency list
struct EdgeNode {
    int dest;
    int cost;
    EdgeNode* next;
    int originalCost;
    bool up;
};

// Represents a router (node) in the graph
struct Router {
    string name;
    bool up = true;
    EdgeNode* head = nullptr;
    float x = 0, y = 0;
    Router() {}
    ~Router() {
        EdgeNode* current = head;
        while (current != nullptr) {
            EdgeNode* next = current->next;
            delete current;
            current = next;
        }
        head = nullptr;
    }
    // Disable copy constructor and copy assignment operator
    // This prevents accidental shallow copies of 'head' and potential double-frees.
    // If deep copying is ever needed, these must be implemented carefully.
    Router(const Router&) = delete;
    Router& operator=(const Router&) = delete;

    Router& operator=(Router&& other) {
        if (this != &other) {
            EdgeNode* current = head;
            while (current != nullptr) {
                EdgeNode* next = current->next;
                delete current;
                current = next;
            }
            name = move(other.name);
            up = other.up;
            head = other.head;
            x = other.x;
            y = other.y;

            other.head = nullptr;
        }
        return *this;
    }
};

// Struct to hold the results of Dijkstra's algorithm
struct DijkstraResult {
    vector<int> dist; // Shortest distance from source to each node
    vector<int> prev; // Previous node in the shortest path
};

// Graph class representing the network topology
class Graph {
public:
    Router nodes[MAX_V];
    int V = 0;
    bool silentMode = false;

    Graph() {}
    ~Graph() {}
    void setSilentMode(bool silent) {
        silentMode = silent;
    }
    void arrangePositions() {
        if (V == 0) return;
        for (int i = 0; i < V; ++i) {
            float angle = 2 * PI * i / V;
            nodes[i].x = GRAPH_CENTER_X + GRAPH_RADIUS * cos(angle);
            nodes[i].y = GRAPH_CENTER_Y + GRAPH_RADIUS * sin(angle);
        }
    }
    int getRouterIndex(const string& name) {
        for (int i = 0; i < V; ++i) {
            if (nodes[i].name == name) {
                return i;
            }
        }
        return -1;
    }
    void addRouter(const string& name = "") {
        if (V >= MAX_V) {
            if (!silentMode) cout << "Max routers reached (" << MAX_V << "). Cannot add more routers.\n";
            return;
        }
        string routerName = name.empty() ? "R" + to_string(V) : name;
        if (getRouterIndex(routerName) != -1) {
            if (!silentMode) cout << "Router " << routerName << " already exists.\n";
            return;
        }
        nodes[V].name = routerName;
        nodes[V].up = true;
        nodes[V].head = nullptr;

        V++; // Increment the count of active routers
        arrangePositions(); // Recalculate positions for all routers
        if (!silentMode) cout << "Router " << routerName << " added.\n";
    }

    // Removes a router from the graph
    void removeRouter(const string& name) {
        int idx = getRouterIndex(name);
        if (idx == -1) {
            cerr << "Error: Router '" << name << "' does not exist.\n";
            return;
        }
        // 1. Remove all incoming edges to 'idx' from other routers
        for (int i = 0; i < V; ++i) {
            if (i == idx) continue; // Skip the router being removed
            removeEdge(nodes[i].name, name, false);
        }
        // 2. Shift remaining routers to fill the gap
        for (int i = idx; i < V - 1; ++i) {
            nodes[i] = move(nodes[i + 1]);
        }
        // Decrement the count of active routers
        V--;
        arrangePositions(); // Recalculate positions
        cout << "Router " << name << " removed.\n";
    }

    // Adds a new link or updates the cost of an existing link between two routers
    void addEdge(const string& fromName, const string& toName, int cost) {
        int u = getRouterIndex(fromName);
        int v = getRouterIndex(toName);

        if (u == -1 || v == -1) {
            if (!silentMode) cout << "Invalid router names.\n";
            return;
        }
        if (u == v) {
            if (!silentMode) cout << "Cannot add link from a router to itself.\n";
            return;
        }

        // Check if an edge already exists and update its cost
        if (updateEdgeCost(u, v, cost, true)) { // 'true' to overwrite original cost
            if (!silentMode) cout << "Updated link cost between " << fromName << " and " << toName << " to " << cost << " ms.\n";
            return;
        }

        // If no edge exists, create new EdgeNode objects for both directions
        EdgeNode* node = new EdgeNode{ v, cost, nodes[u].head, cost, true };
        nodes[u].head = node;

        EdgeNode* node2 = new EdgeNode{ u, cost, nodes[v].head, cost, true };
        nodes[v].head = node2;

        if (!silentMode) cout << "Link added between " << fromName << " and " << toName << " with cost " << cost << " ms.\n";
    }

    // Helper to update the cost of an existing edge
    bool updateEdgeCost(int u, int v, int newCost, bool overwriteOriginal = false) {
        bool updated = false;
        // Update u -> v
        for (auto* edge = nodes[u].head; edge; edge = edge->next) {
            if (edge->dest == v) {
                edge->cost = newCost;
                if (overwriteOriginal) edge->originalCost = newCost;
                edge->up = (newCost != INT_MAX); // Link is up if cost is not INT_MAX
                updated = true;
                break; // Found and updated, can exit loop
            }
        }
        // Update v -> u (symmetric)
        for (auto* edge = nodes[v].head; edge; edge = edge->next) {
            if (edge->dest == u) {
                edge->cost = newCost;
                if (overwriteOriginal) edge->originalCost = newCost;
                edge->up = (newCost != INT_MAX);
                updated = true;
                break; // Found and updated, can exit loop
            }
        }
        return updated;
    }

    // Removes a link between two routers
    void removeEdge(const string& fromName, const string& toName, bool printMessage = true) {
        int u = getRouterIndex(fromName);
        int v = getRouterIndex(toName);

        if (u == -1 || v == -1) {
            if (printMessage) cout << "Invalid router names.\n";
            return;
        }

        // Lambda function to remove an edge from an adjacency list
        auto remove = [&](int from, int to) {
            EdgeNode** curr = &nodes[from].head;
            while (*curr) {
                if ((*curr)->dest == to) {
                    EdgeNode* temp = *curr;
                    *curr = (*curr)->next; // Link previous node to next node
                    delete temp;            // Deallocate the removed EdgeNode
                    return;
                }
                curr = &(*curr)->next;
            }
            };

        remove(u, v); // Remove u -> v
        remove(v, u); // Remove v -> u (symmetric)

        if (printMessage) cout << "Link removed between " << fromName << " and " << toName << ".\n";
    }

    // Toggles the UP/DOWN status of a router
    void toggleRouterStatus(const string& name) {
        int idx = getRouterIndex(name);
        if (idx == -1) {
            cout << "Router " << name << " not found.\n";
            return;
        }
        nodes[idx].up = !nodes[idx].up;
        cout << "Router " << name << " is now " << (nodes[idx].up ? "UP" : "DOWN") << ".\n";
    }

    // Toggles the UP/DOWN status of a link
    void toggleLink(const string& fromName, const string& toName, bool up) {
        int u = getRouterIndex(fromName);
        int v = getRouterIndex(toName);
        if (u == -1 || v == -1) {
            cout << "Invalid router names.\n";
            return;
        }

        bool changed = false;
        // Update u -> v
        for (auto* edge = nodes[u].head; edge; edge = edge->next) {
            if (edge->dest == v) {
                edge->cost = up ? edge->originalCost : INT_MAX; // Set cost to original or INT_MAX
                edge->up = up;
                changed = true;
            }
        }
        // Update v -> u
        for (auto* edge = nodes[v].head; edge; edge = edge->next) {
            if (edge->dest == u) {
                edge->cost = up ? edge->originalCost : INT_MAX;
                edge->up = up;
                changed = true;
            }
        }
        if (!changed) cout << "Link not found.\n";
        else cout << "Link " << fromName << " <--> " << toName << " is now " << (up ? "UP" : "DOWN") << ".\n";
    }

private:
    // Private helper function for Dijkstra's algorithm.
    // Computes shortest distances and predecessors from a source router.
    DijkstraResult runDijkstra(int src) {
        vector<int> dist(V, INT_MAX);
        vector<int> prev(V, -1);
        vector<bool> visited(V, false);

        // Add bounds checking for src, though it should be handled by callers
        if (src < 0 || src >= V) {
            // Return an empty/invalid result if src is out of bounds
            return { vector<int>(), vector<int>() };
        }

        dist[src] = 0;
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<>> pq; // Min-priority queue
        pq.push({ 0, src });

        while (!pq.empty()) {
            int u = pq.top().second;
            pq.pop();

            // If already visited or router is down, skip
            if (visited[u] || !nodes[u].up) continue;
            visited[u] = true;

            // FIX for C6385: Explicitly check dist[u] here before iterating neighbors
            // If dist[u] is INT_MAX, it means 'u' is unreachable from source,
            // so we shouldn't use it to find paths to its neighbors.
            if (dist[u] == INT_MAX) {
                continue;
            }

            // Explore neighbors
            for (EdgeNode* edge = nodes[u].head; edge; edge = edge->next) {
                int v = edge->dest;
                int cost = edge->cost;

                // Check if neighbor is not visited, is up, link is up (cost != INT_MAX)
                // and a shorter path is found.
                // Addition is now safer as dist[u] is guaranteed not to be INT_MAX
                long long new_dist = (long long)dist[u] + cost; // Use long long for safe addition

                // Only update if new_dist is better than current dist[v]
                if (!visited[v] && nodes[v].up && cost != INT_MAX && new_dist < dist[v]) {
                    dist[v] = static_cast<int>(new_dist); // Cast back to int, assuming it fits
                    prev[v] = u;
                    pq.push({ dist[v], v });
                }
            }
        }
        return { dist, prev };
    }

public:
    // Finds and prints the shortest path and its cost between two routers
    int dijkstra(int src, int dest, bool returnCostOnly = false) {
        // Add bounds checking for src and dest
        if (src < 0 || src >= V || dest < 0 || dest >= V) {
            if (!silentMode) cout << "Invalid source or destination router index.\n";
            return INT_MAX;
        }

        DijkstraResult result = runDijkstra(src);
        const auto& dist = result.dist;
        const auto& prev = result.prev;

        // Handle case where runDijkstra returned empty vectors due to invalid src
        if (dist.empty() || prev.empty()) {
            return INT_MAX;
        }

        if (returnCostOnly) {
            return dist[dest]; // Return only the cost
        }

        if (dist[dest] == INT_MAX) {
            cout << "No path from " << nodes[src].name << " to " << nodes[dest].name << ".\n";
            return INT_MAX;
        }

        stack<int> pathStack;
        int current = dest;
        // Reconstruct path from destination back to source
        while (current != -1 && current != src) {
            pathStack.push(current);
            current = prev[current];
        }
        // If current is -1, it means path reconstruction failed (e.g., no path to src, but dist[dest] wasn't INT_MAX)
        // This check ensures src is actually reachable and in the path.
        if (current == -1 && src != dest) { // Added src != dest check for the case when dest is src
            cout << "Error: Path reconstruction failed for " << nodes[src].name << " to " << nodes[dest].name << ".\n";
            return INT_MAX;
        }
        pathStack.push(src); // Add source to the path

        cout << "Shortest path: ";
        vector<int> pathVector; // Used for visualization, could be removed if not strictly needed here
        while (!pathStack.empty()) {
            int nodeIndex = pathStack.top();
            pathVector.push_back(nodeIndex); // For potential external use
            cout << nodes[nodeIndex].name;
            pathStack.pop();
            if (!pathStack.empty()) {
                cout << " -> ";
            }
        }
        cout << "\nTotal Cost: " << dist[dest] << " ms\n";
        return dist[dest];
    }

    // Returns the shortest path as a vector of router indices
    vector<int> findShortestPath(int src, int dest) {
        // Add bounds checking for src and dest
        if (src < 0 || src >= V || dest < 0 || dest >= V) {
            return vector<int>(); // Return empty path
        }

        DijkstraResult result = runDijkstra(src);
        const auto& dist = result.dist;
        const auto& prev = result.prev;

        if (dist.empty() || prev.empty() || dist[dest] == INT_MAX) {
            return vector<int>(); // No path found or invalid source
        }

        stack<int> pathStack;
        int current = dest;
        while (current != -1 && current != src) {
            pathStack.push(current);
            current = prev[current];
        }
        if (current == -1 && src != dest) { // Path reconstruction failed
            return vector<int>();
        }
        pathStack.push(src); // Add source to the path

        vector<int> pathVector;
        while (!pathStack.empty()) {
            pathVector.push_back(pathStack.top());
            pathStack.pop();
        }
        return pathVector;
    }

    // Determines the next hop router from source to destination
    int getNextHop(int src, int dest) {
        // Add bounds checking for src and dest
        if (src < 0 || src >= V || dest < 0 || dest >= V) {
            return -1; // Invalid source or destination
        }

        DijkstraResult result = runDijkstra(src);
        const auto& dist = result.dist;
        const auto& prev = result.prev;

        if (dist.empty() || prev.empty() || dist[dest] == INT_MAX) {
            return -1; // No path
        }
        if (src == dest) {
            return src; // Next hop from a router to itself is itself
        }

        // Trace back from dest to find the node directly connected to src
        int current = dest;
        // Keep tracing back until we find the node whose predecessor is 'src'
        // or until we reach 'src' itself (meaning dest is directly connected).
        while (prev[current] != src && prev[current] != -1 && current != src) {
            current = prev[current];
        }

        // If we found a node whose predecessor is 'src', that node is the next hop.
        if (prev[current] == src) {
            return current;
        }
        // If 'current' is now 'dest' and prev[dest] is 'src', it means 'dest' is directly connected.
        if (current == dest && prev[dest] == src) {
            return dest; // Direct link, next hop is the destination itself
        }

        // This case should ideally not be hit if a path exists and src != dest
        // It might happen if the path is found but the trace-back logic fails for some edge case.
        return -1;
    }

    // Prints the routing table for a given router
    void printRoutingTable(int routerIdx) {
        if (routerIdx < 0 || routerIdx >= V) {
            cout << "Invalid router index.\n";
            return;
        }
        cout << "Routing Table for " << nodes[routerIdx].name << ":\n";
        cout << "Destination\tNext Hop\tCost\n";
        for (int dest = 0; dest < V; ++dest) {
            if (dest == routerIdx) continue; // Skip itself
            int nextHop = getNextHop(routerIdx, dest);
            int cost = dijkstra(routerIdx, dest, true); // Get cost only

            if (cost == INT_MAX) {
                cout << nodes[dest].name << "\t\t" << "-" << "\t\t" << "INF\n";
            }
            // FIX: Changed 'src' to 'routerIdx' as 'src' is not defined in this scope.
            else if (nextHop == -1 || nextHop == routerIdx) { // If nextHop is -1 or the router itself (meaning direct link)
                // For direct links, the next hop is the destination itself
                cout << nodes[dest].name << "\t\t" << nodes[dest].name << "\t\t" << cost << "\n";
            }
            else {
                cout << nodes[dest].name << "\t\t" << nodes[nextHop].name << "\t\t" << cost << "\n";
            }
        }
    }

    // Saves the current network topology to a file
    void saveToFile(const string& filename) {
        ofstream fout(filename);
        if (!fout) {
            cout << "Failed to open file for writing.\n";
            return;
        }

        // Write number of routers
        fout << V << "\n";
        // Write router details (name, up status, position)
        for (int i = 0; i < V; ++i) {
            fout << nodes[i].name << " " << nodes[i].up << " " << nodes[i].x << " " << nodes[i].y << "\n";
        }
        // Write edge details (only once for symmetric links)
        for (int i = 0; i < V; ++i) {
            for (EdgeNode* e = nodes[i].head; e; e = e->next) {
                // Only write if 'i' is less than 'e->dest' to avoid duplicates (e.g., R0-R1 and R1-R0)
                if (i < e->dest) {
                    fout << nodes[i].name << " " << nodes[e->dest].name << " " << e->originalCost << "\n"; // Save original cost
                }
            }
        }
        cout << "Network topology saved to " << filename << ".\n";
    }

    // Loads a network topology from a file
    void loadFromFile(const string& filename) {
        ifstream fin(filename);
        if (!fin) {
            cout << "Failed to open file for reading.\n";
            return;
        }

        // Clear the current graph completely before loading new data
        // Explicitly trigger destructor for each router's edges
        for (int i = 0; i < V; ++i) {
            clearEdges(i); // Ensures all EdgeNodes are deleted
        }
        V = 0; // Reset active router count

        int n_routers;
        fin >> n_routers;
        if (fin.fail() || n_routers < 0) {
            cout << "Error reading number of routers from file. File might be corrupted or empty.\n";
            fin.close();
            return;
        }
        if (n_routers > MAX_V) {
            cout << "Error: Number of routers in file (" << n_routers << ") exceeds MAX_V (" << MAX_V << "). Aborting load.\n";
            fin.close();
            return;
        }

        // Temporarily set silent mode to prevent many console messages during load
        bool originalSilentMode = silentMode;
        setSilentMode(true);

        // Read and add routers
        for (int i = 0; i < n_routers; ++i) {
            string name;
            int upFlag;
            float x, y;
            fin >> name >> upFlag >> x >> y;
            if (fin.fail()) {
                cout << "Error reading router data for router " << i << ". Aborting load.\n";
                setSilentMode(originalSilentMode);
                fin.close();
                return;
            }
            addRouter(name); // This increments V and initializes the new router
            nodes[i].up = (upFlag == 1);
            nodes[i].x = x;
            nodes[i].y = y;
        }

        // Read and add edges
        string from, to;
        int cost;
        while (fin >> from >> to >> cost) {
            if (fin.fail()) break; // End of file or invalid data
            addEdge(from, to, cost); // This adds the edge and sets up/down state based on cost
        }

        setSilentMode(originalSilentMode); // Restore original silent mode
        cout << "Network topology loaded from " << filename << ".\n";
        arrangePositions(); // Re-arrange positions in case old positions were bad
        fin.close();
    }

    // Helper to clear all outgoing edges for a given router index
    void clearEdges(int routerIdx) {
        EdgeNode* curr = nodes[routerIdx].head;
        while (curr) {
            EdgeNode* tmp = curr;
            curr = curr->next;
            delete tmp; // Deallocate the EdgeNode
        }
        nodes[routerIdx].head = nullptr; // Set head to null after clearing
    }

    // Draws the graph (routers, links, costs, shortest path) on the SFML window
    void draw(RenderWindow& window, Font& font, int selectedSource, int selectedDest, const vector<int>& shortestPath) {
        // Draw edges
        for (int i = 0; i < V; ++i) {
            // Only draw links from active (UP) routers
            if (!nodes[i].up) continue;

            for (EdgeNode* e = nodes[i].head; e; e = e->next) {
                // Draw each link only once (e.g., R0-R1, not R1-R0)
                // Also, only draw if the destination router is UP and the link itself is UP
                if (i < e->dest && nodes[e->dest].up && e->cost != INT_MAX) {
                    Vertex line[] = {
                        Vertex(Vector2f(nodes[i].x, nodes[i].y), Color::Black),
                        Vertex(Vector2f(nodes[e->dest].x, nodes[e->dest].y), Color::Black)
                    };
                    window.draw(line, 2, Lines);

                    // Draw cost text at the midpoint of the link
                    float mx = (nodes[i].x + nodes[e->dest].x) / 2;
                    float my = (nodes[i].y + nodes[e->dest].y) / 2;
                    Text costText(std::to_string(e->cost), font, 18);
                    costText.setFillColor(Color(25, 25, 112)); // Dark blue
                    costText.setPosition(mx, my);
                    window.draw(costText);
                }
            }
        }

        // Highlight shortest path in red
        if (!shortestPath.empty()) {
            for (size_t i = 0; i + 1 < shortestPath.size(); ++i) {
                int u = shortestPath[i];
                int v = shortestPath[i + 1];
                Vertex line[] = {
                    Vertex(Vector2f(nodes[u].x, nodes[u].y), Color::Red),
                    Vertex(Vector2f(nodes[v].x, nodes[v].y), Color::Red)
                };
                window.draw(line, 2, Lines);
            }
        }

        // Draw routers (nodes)
        for (int i = 0; i < V; ++i) {
            CircleShape circle(ROUTER_RADIUS);
            circle.setPosition(nodes[i].x - ROUTER_RADIUS, nodes[i].y - ROUTER_RADIUS);

            // Set color based on router status and selection
            if (i == selectedSource) circle.setFillColor(Color::Blue);
            else if (i == selectedDest) circle.setFillColor(Color::Magenta);
            else circle.setFillColor(nodes[i].up ? Color(180, 155, 220) : Color(150, 150, 150)); // Purple if up, grey if down

            window.draw(circle);

            // Draw router name
            Text nameText(nodes[i].name, font, 16);
            nameText.setFillColor(Color::Black);
            // Center the text on the router circle
            FloatRect textRect = nameText.getLocalBounds();
            nameText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            nameText.setPosition(nodes[i].x, nodes[i].y);
            window.draw(nameText);
        }
    }

    // Exports the routing table of a specific router to a file
    void exportRoutingTable(int routerIdx, const string& filename) {
        ofstream fout(filename);
        if (!fout) {
            cout << "Failed to open file for writing.\n";
            return;
        }
        if (routerIdx < 0 || routerIdx >= V) {
            cout << "Invalid router index.\n";
            return;
        }

        fout << "Routing Table for " << nodes[routerIdx].name << ":\n";
        fout << "Destination\tNext Hop\tCost\n";
        for (int dest = 0; dest < V; ++dest) {
            if (dest == routerIdx) continue;
            int nextHop = getNextHop(routerIdx, dest);
            int cost = dijkstra(routerIdx, dest, true); // Get cost only

            if (cost == INT_MAX) fout << nodes[dest].name << "\t\t" << "-" << "\t\t" << "INF\n";
            // Corrected condition: nextHop is either -1 (no path/issue) or the destination itself (direct link)
            else if (nextHop == -1 || nextHop == dest) { // If nextHop is -1 or the destination itself (meaning direct link)
                fout << nodes[dest].name << "\t\t" << nodes[dest].name << "\t\t" << cost << "\n"; // Direct link
            }
            else {
                fout << nodes[dest].name << "\t\t" << nodes[nextHop].name << "\t\t" << cost << "\n";
            }
        }
        cout << "Routing table exported to " << filename << ".\n";
    }
};

// Prints the interactive menu options
void printMenu() {
    cout << "\nMenu:\n";
    cout << "1. Add Router\n";
    cout << "2. Remove Router\n";
    cout << "3. Add/Update Link\n";
    cout << "4. Remove Link\n";
    cout << "5. Toggle Router UP/DOWN\n";
    cout << "6. Find Shortest Path (using selected nodes in GUI)\n";
    cout << "7. Show Routing Table\n";
    cout << "8. Save Topology\n";
    cout << "9. Load Topology\n";
    cout << "0. Exit\n";
    cout << "-----------------------------------\n";
    cout << "Click routers in window to select source and destination for shortest path visualization.\n";
    cout << "Enter your choice: ";
}

int main() {
    cout << "WELCOME TO THE NETWORK ROUTE EXPLORER PROGRAM\n";
    cout << "--------------------------------------------------\n";
    cout << "This program allows you to visualize packet routing in a network of routers.\n";
    cout << "You can add/remove routers, add/remove links, and find the shortest path between routers.\n";
    cout << "You can also save and load the network topology.\n";
    cout << "--------------------------------------------------\n";

    printMenu();

    RenderWindow window(VideoMode(800, 600), "Packet Routing Visualization");
    window.setFramerateLimit(60); // Limit frame rate for smoother operation

    // Load background texture
    Texture backgroundTexture;
    Sprite backgroundSprite;
    if (!backgroundTexture.loadFromFile("background3.jpg")) {
        cout << "Warning: Could not load background image 'background3.jpg'. Using default background.\n";
    }
    else {
        backgroundSprite.setTexture(backgroundTexture);
        // Scale background to window size
        Vector2u textureSize = backgroundTexture.getSize();
        Vector2u windowSize = window.getSize();
        float scaleX = (float)windowSize.x / textureSize.x;
        float scaleY = (float)windowSize.y / textureSize.y;
        backgroundSprite.setScale(scaleX, scaleY);
    }

    Font font;
    if (!font.loadFromFile("Arial.ttf")) {
        cout << "Failed to load font 'Arial.ttf'. Ensure it's in the same directory.\n";
        return 1;
    }

    Graph graph; // Create the graph object

    // Enable silent mode to prevent initial messages from automatic setup
    graph.setSilentMode(true);

    // Initially add 5 routers and some default links
    for (int i = 0; i < 5; ++i) {
        graph.addRouter();
    }

    // Add some initial links
    graph.addEdge("R0", "R1", 10);
    graph.addEdge("R1", "R2", 15);
    graph.addEdge("R2", "R3", 20);
    graph.addEdge("R3", "R4", 25);
    graph.addEdge("R0", "R4", 50);
    graph.addEdge("R1", "R3", 30);

    // Disable silent mode for user interactions
    graph.setSilentMode(false);

    int selectedSource = -1; // Index of the currently selected source router
    int selectedDest = -1;   // Index of the currently selected destination router
    vector<int> shortestPath; // Stores the indices of routers in the shortest path

    // Main SFML window loop
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window)); // Get mouse position in world coordinates
                for (int i = 0; i < graph.V; ++i) {
                    // Calculate distance from mouse click to router center
                    float dx = mousePos.x - graph.nodes[i].x;
                    float dy = mousePos.y - graph.nodes[i].y;
                    // Check if click is within the router circle
                    if (dx * dx + dy * dy <= ROUTER_RADIUS * ROUTER_RADIUS) {
                        if (selectedSource == -1) {
                            selectedSource = i;
                            cout << "Selected source: " << graph.nodes[i].name << "\n";
                        }
                        else if (selectedDest == -1 && i != selectedSource) {
                            selectedDest = i;
                            cout << "Selected destination: " << graph.nodes[i].name << "\n";

                            shortestPath = graph.findShortestPath(selectedSource, selectedDest);
                            if (shortestPath.empty()) {
                                cout << "No path found between " << graph.nodes[selectedSource].name << " and " << graph.nodes[selectedDest].name << ".\n";
                            }
                            else {
                                cout << "Shortest path: ";
                                for (int node : shortestPath) {
                                    cout << graph.nodes[node].name << " ";
                                }
                                cout << "\n";
                            }
                        }
                        else {
                            
                            selectedSource = i;
                            selectedDest = -1; // Reset destination
                            shortestPath.clear(); // Clear old path
                            cout << "Selection reset. New source: " << graph.nodes[i].name << "\n";
                        }
                        break; 
                    }
                }
            }
        }
        window.clear();
        if (backgroundTexture.getSize().x > 0) { 
            window.draw(backgroundSprite);
        }

        // Draw the graph elements
        graph.draw(window, font, selectedSource, selectedDest, shortestPath);

        // Display the window contents
        window.display();

        // Handle keyboard input from console 
        if (_kbhit()) {
            char ch = _getch(); ;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');


            switch (ch) {
            case '1': {
                string name;
                cout << "Enter router name (or press Enter for default 'R#'): ";
                getline(cin, name);
                graph.addRouter(name);
                break;
            }
            case '2': {
                string name;
                cout << "Enter router name to remove: ";
                getline(cin, name);
                graph.removeRouter(name);
                selectedSource = selectedDest = -1;
                shortestPath.clear();
                break;
            }
            case '3': {
                string from, to;
                int cost;
                cout << "Enter source router name: ";
                getline(cin, from);
                cout << "Enter destination router name: ";
                getline(cin, to);
                cout << "Enter link cost (in ms): ";
                string cost_str;
                getline(cin, cost_str);
                try {
                    cost = stoi(cost_str);
                    graph.addEdge(from, to, cost);
                }
                catch (const std::invalid_argument& e) {
                    cout << "Invalid cost entered. Please enter a number.\n";
                }
                catch (const std::out_of_range& e) {
                    cout << "Cost value out of integer range.\n";
                }
                shortestPath.clear();
                break;
            }
            case '4': {
                string from, to;
                cout << "Enter source router name: ";
                getline(cin, from);
                cout << "Enter destination router name: ";
                getline(cin, to);
                graph.removeEdge(from, to);
                shortestPath.clear();
                break;
            }
            case '5': {
                string name;
                cout << "Enter router name to toggle UP/DOWN: ";
                getline(cin, name);
                graph.toggleRouterStatus(name);
                shortestPath.clear(); // Path might change
                break;
            }
            case '6': {
                if (selectedSource != -1 && selectedDest != -1) {
                    shortestPath = graph.findShortestPath(selectedSource, selectedDest);
                    if (shortestPath.empty()) {
                        cout << "No path found.\n";
                    }
                    else {
                        cout << "Shortest path: ";
                        for (int node : shortestPath) cout << graph.nodes[node].name << " ";
                        cout << "\n";
                    }
                }
                else {
                    cout << "Please select source and destination routers by clicking in the window first.\n";
                }
                break;
            }
            case '7': {
                string name;
                cout << "Enter router name to show routing table: ";
                getline(cin, name);
                int idx = graph.getRouterIndex(name);
                if (idx == -1) cout << "Router not found.\n";
                else graph.printRoutingTable(idx);
                break;
            }
            case '8': {
                string filename;
                cout << "Enter filename to save topology (e.g., network.txt): ";
                getline(cin, filename);
                graph.saveToFile(filename);
                break;
            }
            case '9': {
                string filename;
                cout << "Enter filename to load topology (e.g., network.txt): ";
                getline(cin, filename);
                int old_V = graph.V;
                graph.loadFromFile(filename);

                selectedSource = -1;
                selectedDest = -1;
                shortestPath.clear();
                break;
            }
            case '0': {
                window.close(); // Close SFML window
                cout << "Exiting program. Goodbye!\n";
                break;
            }
            default:
                cout << "Invalid option. Please choose from the menu (1-9 or 0 to exit).\n";
                break;
            }
            if (window.isOpen()) {
                printMenu();
            }
        }
    }

    return 0;
}
