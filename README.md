# Network-Route-Explorer

 Network Route Explorer Program - Technical Report
 1. Introduction
The Network Route Explorer Program is a comprehensive tool designed to visualize and analyze packet routing in computer networks. This program provides both a graphical interface and a console-based menu system that allows users to create, manage, and analyze network topologies with intuitive visualization of routing paths.
2. System Architecture
2.1 DATA REPRESENTATION:

The program models a computer network as an undirected weighted graph where:
•	Network routers are represented as vertices
•	Network links between routers are represented as edges with associated latency costs
•	The graph maintains both current and original link states to support failure simulation

2.2 CORE COMPONENTS
1. Network Management Core:
•	Handles all router and link operations
•	Maintains network state and topology
•	Implements routing calculations

2. Visualization Engine:
•	Renders the network topology with status indicators
•	Provides interactive router selection
•	Displays calculated paths with visual distinction

3. User Interface System:
•	Console menu for command input
•	Graphical display for visual feedback
•	Integrated input handling for both interfaces
3. Key Data Structures

Our program uses several important data structures to manage and analyze the network:
1.	Arrays: We use a simple array to hold all the routers in our network. Think of it like numbered slots where each router sits, letting us quickly find any router by its number.
2.	Linked Lists (Adjacency Lists): To show how routers are connected, we use linked lists. Each router has its own little list that points to all the other routers it's directly connected to. This is efficient because we only store connections that actually exist.
3.	Vectors: These are like smart, flexible arrays from the C++ standard library. We use them in our shortest path algorithm to keep track of: 
•	Distances: How far each router is from our starting point.
•	Previous Routers: Which router we came from to reach the current one on the shortest path.
•	Visited Status: Whether we've already checked a router during our calculations.
4.	Priority Queue: This is a special list that always keeps the "best" item at the top. In our case, it helps Dijkstra's algorithm always pick the next closest router to explore, making the pathfinding super-efficient.
5.	Stack: When we find a shortest path, we use a stack to help us list the routers in the correct order from start to finish. It's like putting plates on top of each other and then taking them off one by one.
These data structures work together to make sure our network can be built, changed, and analyzed quickly and accurately.
4. Key Algorithms

 4.1 PATH CALCULATION ALGORITHM

The program employs an optimized shortest path algorithm based on Dijkstra's approach with these characteristics:
•	Priority queue implementation for efficient path finding
•	Dynamic adjustment for router/link availability
•	Support for both path cost and full path reconstruction
•	Efficient handling of network changes

 4.2 ROUTING TABLE GENERATION
The routing table system:
•	Calculates optimal paths to all destinations from a given router
•	Determines next-hop routers for each destination
•	Handles network partitions gracefully
•	Supports both console display and file export
 5. User Interface
 5.1 VISUAL INTERFACE FEATURES

The graphical presentation provides:
•	Clear router representation with status indication (up/down)
•	Visible link costs between routers
•	Color-coded path visualization
•	Interactive selection of source and destination routers
•	Immediate feedback for path calculations

5.2 CONSOLE INTERFACE FEATURES

The text interface offers:
•	Comprehensive menu system
•	Detailed routing information display
•	Network modification commands
•	File operations for topology persistence
•	Clear status and error messages
6. Data Persistence
The program implements a complete file I/O system that:
•	Saves all network topology information
•	Preserves router positions and states
•	Maintains link configurations and costs
•	Supports complete network restoration from file
7. Operational Characteristics
7.1 NETWORK MANAGEMENT

The system supports:
•	Dynamic addition/removal of routers
•	Link configuration and modification
•	Simulation of network failures
•	Real-time topology changes

7.2 PERFORMANCE CONSIDERATIONS

The implementation focuses on:
•	Efficient path calculation for interactive use
•	Responsive graphical display
•	Scalable data structures
•	Balanced resource usage
 8. Limitations
Current limitations include:
•	Fixed upper limit on network size
•	Basic visualization capabilities
•	Simplified network model
•	Limited error recovery
 9. Enhancement Opportunities
Potential future improvements:
•	Support for larger network sizes
•	Additional routing protocol simulations
•	Enhanced visualization features
•	Network traffic simulation
•	Advanced failure scenarios
 10. Conclusion

The Network Route Explorer Program provides an effective platform for network routing visualization and analysis. Its dual-interface approach combines the clarity of visual representation with the precision of console controls, making it suitable for educational and professional use. The system successfully demonstrates fundamental networking concepts while providing practical tools for network planning and troubleshooting.

The program's architecture allows for future expansion while maintaining current usability, positioning it as a valuable resource for understanding network routing principles and operations.
