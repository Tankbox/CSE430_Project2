#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <string>
#include "Project2.h"

using namespace std;

#define DEBUG true

void debugger(GENI myGeni, RequestList myRequestList)
{
	for (int i = 0; i < myGeni.vertices.size(); ++i)
	{
		printf("\nNode %d has degree %d with these neighbors:\n\t", myGeni.vertices.at(i).name, myGeni.vertices.at(i).neighbors.size());
		for (int j = 0; j < myGeni.vertices.at(i).neighbors.size(); ++j)
		{
			if (j+1 != myGeni.vertices.at(i).neighbors.size())
				printf("%d, ", myGeni.vertices.at(i).neighbors.at(j)->name);
			else
				printf("%d", myGeni.vertices.at(i).neighbors.at(j)->name);
		}
	}

	printf("\n\n");

	for (int i = 0; i < myGeni.vertices.size(); ++i)
		printf("Node %d:\n\tVMs:\t%d\n\tFTEs\t%d\n", myGeni.getName(i), myGeni.vertices.at(i).VM, myGeni.vertices.at(i).FTE);

	printf("\n");

	for (int i = 0; i < myGeni.connections.size(); ++i)
		printf("Connection from %d to %d bandwidth:\t%d\n", 
			myGeni.connections.at(i).nodes[0]->getName(),
			myGeni.connections.at(i).nodes[1]->getName(),
			myGeni.connections.at(i).bandwidth);
}

void readGraph(GENI &myGeni, const char* graphFile)
{
	NeighborList myNeighborList;
	// Open the graphFile 
	ifstream myFile(graphFile);

	// Get the number of vertices that are in this GENI network
	int vertices = -1;
	myFile >> vertices;

	// Create the initial list of vertices
	for (int i = 0; i < vertices; ++i)
	{
		Vertex myVertex;
		myVertex.name = i+1;
		myGeni.vertices.push_back(myVertex);
	}

	// Create the neighor list (this is to be used later)
	for (int i = 0; i < vertices; ++i)
	{
		// Create a list of neighbors for one node
		Neighbors myNeighbors;	
		int degree = -1;
		myFile >> degree;
		for (int j = 0; j < degree; ++j)
		{					
			// This is one neighbor (of one to many)
			int aNeighbor = -1;
			myFile >> aNeighbor;
			myNeighbors.name.push_back(aNeighbor);			
		}
		// Save the neighbors for a node to the neighbor list
		myNeighborList.neighbors.push_back(myNeighbors);
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
		myConnection.nodes[0] = &myGeni.vertices.at(connectionPoint-1);
		// Get the connection point for the second node
		if (!(myFile >> connectionPoint))
			break;
		myConnection.nodes[1] = &myGeni.vertices.at(connectionPoint-1);
		// Get the bandwidth for this connection
		if (!(myFile >> bandwidth))
			break;
		myConnection.bandwidth = bandwidth;

		// Push back the new connection to the list of connections
		myGeni.connections.push_back(myConnection);
		
	} while (!myFile.eof());

	// Read the degrees for all the nodes and all their neighbors
	for (int i = 0; i < myNeighborList.neighbors.size(); ++i)
	{
		// Read each neighbor and push it back to the neighbors vector
		for (int j = 0; j < myNeighborList.neighbors.at(i).name.size(); ++j)
		{
			vertex* newNeighbor = NULL;
			newNeighbor = &myGeni.vertices.at(myNeighborList.neighbors.at(i).name.at(j)-1);
			myGeni.vertices.at(i).neighbors.push_back(newNeighbor);
		}
	}

	// Close the graph.txt file
	myFile.close();
	// Return the GENI network that has been built
}

RequestList readRequests(const char* requestsFile)
{
	RequestList myRequestList;
	// Open the requestsFile
	ifstream myFile(requestsFile);

	// Save the number of requests that are to be made
	int numRequests = -1;
	myFile >> numRequests;

	// Save each request and add it to the RequestList data structure
	for (int i = 0; i < numRequests; ++i)
	{
		Request myRequest;
		myFile >> myRequest.source;
		myFile >> myRequest.destination;
		myRequestList.requests.push_back(myRequest);
	}

	// Close the requests.txt file
	myFile.close();
	// File successfully read
	return myRequestList;
}

void pingRequest(GENI &myGeni, Request myRequest)
{

}

int main( int argc, const char* argv[] )
{
	GENI myGeni;
	// Read in the graphs.txt, this should be your first parameter
	readGraph(myGeni, argv[1]);
	// Read in the requests.txt, this should be your second parameter
	RequestList myRequestList = readRequests(argv[2]);

	if (DEBUG)
		debugger(myGeni, myRequestList);

	for (int i = 0; i < myRequestList.requests.size(); ++i)
	{
		pingRequest(myGeni, myRequestList.requests.at(i));
	}
	return 0;
}