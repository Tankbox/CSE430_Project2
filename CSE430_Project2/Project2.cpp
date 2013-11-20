#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <string>
#include <thread>
#include <random>
#include <chrono>
#include <time.h>
#include <iostream>
#include "Project2.h"

using namespace std;

// True = program in debug mode (prints status of graph)
// False = program not in debug mode
#define DEBUG false

#define BANDWIDTH_ERROR -2
#define GO_BACK -1
#define VM_ERROR 0
#define FTE_ERROR 1
#define BANDWIDTH_ALLOCATED 2
#define DESTINATION_REACHED 3
#define PING_FAILED 4

// Debugger function to simply print out the current state of the GENI network
// and the current state of the connections
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

void printPing(Request myRequest, vector<int> visited, int outcome)
{
	switch(outcome) {
	case DESTINATION_REACHED:
		printf("\nPing from %d to %d was successful.\nThis is the trace:\n", myRequest.source, myRequest.destination);
		for(int i = 0; i < visited.size(); ++i)
			printf("%d ", visited.at(i));
		break;
	case PING_FAILED:
		printf("\nPing from %d to %d failed.\nThis is the trace:\n", myRequest.source, myRequest.destination);
		for(int i = 0; i < visited.size(); ++i)
			printf("%d ", visited.at(i));
		break;
	}
}

void errorHandler(int error)
{
	// Error handler tells you where the request went wrong
	// and what the problem was
	switch(error) {
	case VM_ERROR:
		printf("There was a VM Error\n");
		exit(EXIT_FAILURE);
		break;
	case FTE_ERROR:
		printf("There was an FTE Error\n");
		exit(EXIT_FAILURE);
		break;
	case BANDWIDTH_ERROR:
		printf("There was a BANDWIDTH Error\n");
		exit(EXIT_FAILURE);
		break;
	}
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

int bandwidth(int currentNode, int nextNode, vector<Connection>* &connections)
{
	// If the currentNode is less than the nextNode, look in the connections vector for
	// nodes[currentNode], nodes[nextNode] and return success if found
	if (currentNode < nextNode)
	{
		for (int i = 0; i < connections->size(); ++i)
		{
			if ((connections->at(i).nodes[0]->getName() == currentNode) && (connections->at(i).nodes[1]->getName() == nextNode))
			{
				connections->at(i).bandwidth -= 5;
				return i;
			}
		}
	}
	// Else the currentNode is greater than the nextNode, look in the connections vector for
	// nodes[nextNode], nodes[currentNode] and return success if found
	else
	{
		for (int i = 0; i < connections->size(); ++i)
		{
			if ((connections->at(i).nodes[0]->getName() == nextNode) && (connections->at(i).nodes[1]->getName() == currentNode))
			{
				connections->at(i).bandwidth -= 5;
				return i;
			}
		}
	}

	return BANDWIDTH_ERROR;
}

int hop(vertex* current, vertex* next, vertex* destination, vector<Connection>* connections, int iter, vector<int> &visited, vector<int> &visitedThisTime)
{
	int connectionLocation = BANDWIDTH_ERROR;
	// If we have not visited this node before, add it to the list
	if (find(visited.begin(), visited.end(), current->getName()) == visited.end())
	{
		// If the FTEs are available, allocate them
		if (current->FTE >= 5)
			current->FTE -= 5;
		// Add this node to the visited lists
		visited.push_back(current->getName());
		visitedThisTime.push_back(current->getName());
	}

	// If the current node is equal to our destination
	if (current->getName() == destination->getName())
	{
		// If the VM is available, allocate it at the destination
		if (current->VM >= 1)
		{
			--current->VM;
			return DESTINATION_REACHED;
		}
		// Else there is an error (VM not available)
		else
			errorHandler(VM_ERROR);
	}
	// Else the current node is not equal to our destination
	else
	{
		// If the next node's name is not in the visited list, hop to it
		if (find(visited.begin(), visited.end(), next->getName()) == visited.end())
		{
			// We are about to jump to a new node, so allocate the bandwidth
			connectionLocation = bandwidth(current->getName(), next->getName(), connections);
			// If there is a bandwidth error, handle it
			if (connectionLocation == BANDWIDTH_ERROR)
				errorHandler(BANDWIDTH_ERROR);
			// Else if the hop takes us to our destination
			else if (hop(next, next->neighbors.at(0), destination, connections, 0, visited, visitedThisTime) == DESTINATION_REACHED)
				return DESTINATION_REACHED;
			// Else we need to give the bandwidth for this hop back
			else
				connections->at(connectionLocation).bandwidth += 5;
		}
		// Else if there are still neighbors to check out, stay on the current node and check them
		else if (iter < current->neighbors.size() - 1)
		{
			if (hop(current, current->neighbors.at(iter), destination, connections, ++iter, visited, visitedThisTime) == DESTINATION_REACHED)
				return DESTINATION_REACHED;
			else
				return GO_BACK;
		}
	}
	current->FTE += 5;
	return GO_BACK;
}

void pingRequest(GENI &myGeni, Request myRequest, default_random_engine &generator)
{
	clock_t start, stop;
	// Start the clock for this request
	start = clock();

	// The vector of visited nodes
	vector<int> visited;
	// Save the old GENI map to restore resources
	GENI oldGeni = myGeni;

	// Allocate VMs at the source (and error checking)
	if (myGeni.vertices.at(myRequest.source-1).VM > 0)
		--myGeni.vertices.at(myRequest.source-1).VM;
	else	
		errorHandler(VM_ERROR);

	// Add the source node to the visited vector
	visited.push_back(myRequest.source);

	// While the end of the visited array is not the destination, hop
	do {
		// Vector for nodes visited for this DFS
		vector<int> visitedThisTime;
		visitedThisTime.push_back(myRequest.source);

		// Secore FTEs at the source node
		if (myGeni.vertices.at(myRequest.source-1).FTE >= 5)
			myGeni.vertices.at(myRequest.source-1).FTE -= 5;
		else
			errorHandler(FTE_ERROR);

		// If the ping was successful, print a successful ping statement
		if(hop(&myGeni.vertices.at(myRequest.source-1), myGeni.vertices.at(myRequest.source-1).neighbors.at(0), &myGeni.vertices.at(myRequest.destination-1), &myGeni.connections, 0, visited, visitedThisTime) == DESTINATION_REACHED)
			printPing(myRequest, visitedThisTime, DESTINATION_REACHED);
		// Else print a failed ping statement
		else
			printPing(myRequest, visitedThisTime, PING_FAILED);
	} while (visited.back() != myRequest.destination);

	// Stop the clock for this request
	stop = clock();
	// Hold the resources for this successful request for a random time between 1 and 5 seconds
	uniform_int_distribution<int> dist(1, 5);
	int hold = dist(generator);
	this_thread::sleep_for(chrono::seconds(hold));

	++myGeni.vertices.at(myRequest.source).VM;
	++myGeni.vertices.at(myRequest.destination).VM;
	
	cout << "\nThis requestion took " + to_string((double(stop - start) / CLOCKS_PER_SEC))
		+ " seconds to complete. The resources were held for " + to_string(hold) + " seconds." << endl;
}

int main( int argc, const char* argv[] )
{
	GENI myGeni;
	// Read in the graphs.txt, this should be your first parameter
	readGraph(myGeni, argv[1]);
	// Read in the requests.txt, this should be your second parameter
	RequestList myRequestList = readRequests(argv[2]);

	// Run debugger to make sure the graph and request list are correct
	if (DEBUG)
		debugger(myGeni, myRequestList);

	default_random_engine generator;
	for (int i = 0; i < myRequestList.requests.size(); ++i)
		pingRequest(myGeni, myRequestList.requests.at(i), generator);

	if (DEBUG)
		debugger(myGeni, myRequestList);

	return 0;
}