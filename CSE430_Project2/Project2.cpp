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
#define ALLOCATE 5
#define DEALLOCATE 6
#define DFS_ERROR 7

// Debugger function to simply print out the current state of the GENI network
// and the current state of the connections
void debugger(GENI myGeni, RequestList myRequestList)
{
	for (int i = 0; i < myGeni.vertices.size(); ++i)
	{
		cout << "\nNode " + to_string(myGeni.vertices.at(i).name) + " has degree " +
			 to_string(myGeni.vertices.at(i).neighbors.size()) + " with these neighbors:\n\t";
		for (int j = 0; j < myGeni.vertices.at(i).neighbors.size(); ++j)
		{
			if (j+1 != myGeni.vertices.at(i).neighbors.size())
				cout << to_string(myGeni.vertices.at(i).neighbors.at(j)->name) + ", ";
			else
				cout << to_string(myGeni.vertices.at(i).neighbors.at(j)->name);
		}
	}

	cout << "\n\n";

	for (int i = 0; i < myGeni.vertices.size(); ++i)
		cout << "Node " + to_string(myGeni.getName(i)) + ":\n\tVMs:\t" + 
			to_string(myGeni.vertices.at(i).VM) + "\n\tFTEs\t" + to_string(myGeni.vertices.at(i).FTE);

	cout << "\n";

	for (int i = 0; i < myGeni.connections.size(); ++i)
		cout << "Connection from " + to_string(myGeni.connections.at(i).nodes[0]->getName()) + 
			" to " + to_string(myGeni.connections.at(i).nodes[1]->getName()) + " bandwidth:\t" + 
			to_string(myGeni.connections.at(i).bandwidth) + "\n";
}

void printPing(Request myRequest, vector<int> visited, int outcome)
{
	switch(outcome) {
	case DESTINATION_REACHED:
		cout << "\nPing from " + to_string(myRequest.source) + " to " + to_string(myRequest.destination)
			+ " was successful.\nThis is the trace:\n";
		for(int i = 0; i < visited.size(); ++i)
			cout << to_string(visited.at(i)) + " ";
		break;
	case PING_FAILED:
		cout << "\nPing from " + to_string(myRequest.source) + " to " + to_string(myRequest.destination)
			+ " failed.\nThis is the trace:\n";
		for(int i = 0; i < visited.size(); ++i)
			cout << to_string(visited.at(i)) + " ";
		break;
	}

	cout << "\n\n";
}

void errorHandler(int error)
{
	// Error handler tells you where the request went wrong
	// and what the problem was
	switch(error) {
	case VM_ERROR:
		cout << "\nThere was a VM Error\n";
		exit(EXIT_FAILURE);
		break;
	case FTE_ERROR:
		cout << "\nThere was an FTE Error\n";
		exit(EXIT_FAILURE);
		break;
	case BANDWIDTH_ERROR:
		cout << "\nThere was a BANDWIDTH Error\n";
		exit(EXIT_FAILURE);
		break;
	case DFS_ERROR:
		cout << "\nThere was a DFS Error\nThe graph tried all the neighbors of the"
			" source node and was unable to find the destination.";
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

int bandwidth(int currentNode, int nextNode, vector<Connection>* connections, int task)
{
	// If the currentNode is less than the nextNode, look in the connections vector for
	// nodes[currentNode], nodes[nextNode] and return success if found
	if (currentNode < nextNode)
	{
		for (int i = 0; i < connections->size(); ++i)
		{
			if ((connections->at(i).nodes[0]->getName() == currentNode) && (connections->at(i).nodes[1]->getName() == nextNode))
			{
				if (task == ALLOCATE)
					connections->at(i).bandwidth -= 5;
				else if (task == DEALLOCATE)
					connections->at(i).bandwidth += 5;
				else
					errorHandler(BANDWIDTH_ERROR);
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
				if (task == ALLOCATE)
					connections->at(i).bandwidth -= 5;
				else if (task == DEALLOCATE)
					connections->at(i).bandwidth += 5;
				else
					errorHandler(BANDWIDTH_ERROR);
				return i;
			}
		}
	}

	return BANDWIDTH_ERROR;
}

int hop(vertex* current, vertex* next, vertex* destination, vector<Connection>* connections, int iter, vector<int> &visitedThisTime)
{
	int connectionLocation = BANDWIDTH_ERROR;
	// If we have not visited this node before, add it to the list
	if (find(visitedThisTime.begin(), visitedThisTime.end(), current->getName()) == visitedThisTime.end())
	{
		// If the FTEs are available, allocate them
		if (current->FTE >= 5)
			current->FTE -= 5;
		// Add this node to the visited lists
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
		if (find(visitedThisTime.begin(), visitedThisTime.end(), next->getName()) == visitedThisTime.end())
		{
			// We are about to jump to a new node, so allocate the bandwidth
			connectionLocation = bandwidth(current->getName(), next->getName(), connections, ALLOCATE);
			// If there is a bandwidth error, handle it
			if (connectionLocation == BANDWIDTH_ERROR)
				errorHandler(BANDWIDTH_ERROR);
			// Else if the hop takes us to our destination
			else if (hop(next, next->neighbors.at(0), destination, connections, 0, visitedThisTime) == DESTINATION_REACHED)
				return DESTINATION_REACHED;
			// Else we need to give the bandwidth for this hop back
			else
				connections->at(connectionLocation).bandwidth += 5;
		}
		// Else if there are still neighbors to check out, stay on the current node and check them
		else if (iter < current->neighbors.size() - 1)
		{
			if (hop(current, current->neighbors.at(iter), destination, connections, ++iter, visitedThisTime) == DESTINATION_REACHED)
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
	int attempt = 0, iter = 0;
	clock_t start, stop;
	// Start the clock for this request
	start = clock();

	// The vector of visited nodes
	Visited visited;
	//vector<int> visited;
	vector<int> visitedThisTime;
	// Save the old GENI map to restore resources
	GENI oldGeni = myGeni;

	// Allocate VMs at the source (and error checking)
	if (myGeni.vertices.at(myRequest.source-1).VM > 0)
		--myGeni.vertices.at(myRequest.source-1).VM;
	else	
		errorHandler(VM_ERROR);

	// While the end of the visited array is not the destination, hop
	do {
		// Vector for nodes visited for this DFS
		visitedThisTime.clear();
		visitedThisTime.push_back(myRequest.source);
		visited.aRoute.push_back(visitedThisTime);

		// Secore FTEs at the source node
		if (myGeni.vertices.at(myRequest.source-1).FTE >= 5)
			myGeni.vertices.at(myRequest.source-1).FTE -= 5;
		else
			errorHandler(FTE_ERROR);

		if (iter == myGeni.vertices.at(myRequest.source-1).neighbors.size())
			errorHandler(DFS_ERROR);

		// If the ping was successful, print a successful ping statement
		if(hop(&myGeni.vertices.at(myRequest.source-1), myGeni.vertices.at(myRequest.source-1).neighbors.at(iter), &myGeni.vertices.at(myRequest.destination-1), &myGeni.connections, iter, visitedThisTime) == DESTINATION_REACHED)
		{	
			// If this route is a new one
			if (find(visited.aRoute.begin(), visited.aRoute.end(), visitedThisTime) == visited.aRoute.end())
			{
				visited.aRoute.at(attempt++) = visitedThisTime;
				printPing(myRequest, visitedThisTime, DESTINATION_REACHED);
				++iter;
			}
			// Else we need to do something here
			else
				++iter;			
		}
		// Else print a failed ping statement
		else
		{
			// If this route is a new one
			if (find(visited.aRoute.begin(), visited.aRoute.end(), visitedThisTime) == visited.aRoute.end())
			{
				visited.aRoute.at(attempt++) = visitedThisTime;
				printPing(myRequest, visitedThisTime, PING_FAILED);
				++iter;
			}
			// Else we need to do something here
			else
				++iter;	
		}
	} while (visited.aRoute.back().back() != myRequest.destination);

	// Stop the clock for this request
	stop = clock();
	// Hold the resources for this successful request for a random time between 1 and 5 seconds
	uniform_int_distribution<int> dist(1, 5);
	int hold = dist(generator);
	//this_thread::sleep_for(chrono::seconds(hold));

	// Deallocation of resources from the successful ping after the hold
	++myGeni.vertices.at(myRequest.source-1).VM;
	++myGeni.vertices.at(myRequest.destination-1).VM;
	for (int i = 0; i < visitedThisTime.size(); ++i)
	{
		if (i+1 != visitedThisTime.size())
		{
			if (visitedThisTime.at(i) < visitedThisTime.at(i+1))
				bandwidth(visitedThisTime.at(i), visitedThisTime.at(i+1), &myGeni.connections, DEALLOCATE);
			else if (visitedThisTime.at(i+1) < visitedThisTime.at(i))
				bandwidth(visitedThisTime.at(i+1), visitedThisTime.at(i), &myGeni.connections, DEALLOCATE);
		}
		myGeni.vertices.at(visitedThisTime.at(i)-1).FTE += 5;
	}
	
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