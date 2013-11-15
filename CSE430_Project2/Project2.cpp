#include <cstdio>
#include <cstdlib>
#include <fstream>

using namespace std;

bool readGraph(const char* graphFile)
{
	// Open the graphFile 
	ifstream myFile(graphFile);

	int vertices;
	myFile >> vertices;

	// Close the graph.txt file
	myFile.close();
	// File successfully read
	return true;
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
	readGraph(argv[1]);
	readRequests(argv[2]);
}