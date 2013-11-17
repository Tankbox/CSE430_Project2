#ifndef _PROJECT2_

#include <vector>

using namespace std;

struct vertex;

typedef struct request {
	int source;
	int destination;
}Request;

typedef struct vertex {
	int name;
	vector<vertex*> neighbors;
	int VM;
	int FTE;

	int getName() { return name; }
}Vertex;

typedef struct connection {
	int bandwidth;
	Vertex* nodes[2];
}Connection;

typedef struct geni {
	vector<Vertex> vertices;
	vector<Connection> connections;
	int getName(int index) { return vertices.at(index).getName(); }
}GENI;

typedef struct requestList {
	vector<Request> requests;
}RequestList;

typedef struct neighbors {
	vector<int> name;
}Neighbors;

typedef struct neighbotList {
	vector<Neighbors> neighbors;
}NeighborList;

#endif