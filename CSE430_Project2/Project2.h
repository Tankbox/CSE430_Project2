#ifndef _PROJECT2_

#include <vector>

using namespace std;

typedef struct vertex {
	int name;
	int degree;
	vector<int> neighbors;
	int VM;
	int FTE;

	int getName() { return name; }
}Vertex;

typedef struct connection {
	int bandwidth;
	int nodes[2];
}Connection;

typedef struct geni {
	vector<Vertex> vertices;
	vector<Connection> connections;

	int getName(int index) { return vertices.at(index).getName(); }
}GENI;

#endif