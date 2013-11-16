#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <string>
#include "Project2.h"

using namespace std;

GENI readGraph(const char* graphFile)
{
	GENI myGeni;
	// Open the graphFile 
	ifstream myFile(graphFile);

	// Get the number of vertices that are in this GENI network
	int vertices = -1;
	myFile >> vertices;

	// Read the degrees for all the nodes and all their neighbors
	for (int i = 0; i < vertices; ++i)
	{
		Vertex myVertex;
		myFile >> myVertex.degree;
		myVertex.name = i+1;
		// Read each neighbor and push it back to the neighbors vector
		for (int j = 0; j < myVertex.degree; ++j)
		{
			int newNeighbor = -1;
			myFile >> newNeighbor;
			myVertex.neighbors.push_back(newNeighbor);
		}
		// Sort the neighbors vector and then add it to the GENI network
		sort(myVertex.neighbors.begin(), myVertex.neighbors.end());
		myGeni.vertices.push_back(myVertex);
	}

	// Read the virtual machines and floating table etries for each node
	for (int i = 0; i < vertices; ++i)
	{
		int VM = -1, FTE = -1;
		myFile >> VM;
		myFile >> FTE;
		myGeni.vertices.at(i).VM = VM;
		myGeni.vertices.at(i).FTE = FTE;
	}

	// Read the connections until we get to the end of the file
	do {
		// Make new variables and and a new connection
		int connectionPoint = -1, bandwidth = -1;
		Connection myConnection;

		// Get the connection point for the first node
		if (!(myFile >> connectionPoint))
			break;
		myConnection.nodes[0] = connectionPoint;
		// Get the connection point for the second node
		if (!(myFile >> connectionPoint))
			break;
		myConnection.nodes[1] = connectionPoint;
		// Get the bandwidth for this connection
		if (!(myFile >> bandwidth))
			break;
		myConnection.bandwidth = bandwidth;

		// Push back the new connection to the list of connections
		myGeni.connections.push_back(myConnection);
		
	} while (!myFile.eof());

	// Close the graph.txt file
	myFile.close();
	// Return the GENI network that has been built
	return myGeni;
}

bool readRequests(const char* requestsFile)
{
	// Open the requestsFile
	ifstream myFile(requestsFile);

	// Close the requests.txt file
	myFile.close();
	// File successfully read
	return true;
}

int main( int argc, const char* argv[] )
{
	// Read in the graphs.txt, this should be your first parameter
	GENI myGeni = readGraph(argv[1]);
	readRequests(argv[2]);
}