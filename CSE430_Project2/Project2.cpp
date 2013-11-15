#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace std;

bool readGraph(const char* graph)
{
	ifstream myFile(*graph, "r");
	
	int vertices;
	cin >> vertices;
	printf("%d", vertices);
	return false;
}

int main( int argc, const char* argv[] )
{
	readGraph(argv[0]);
	printf( "\nHello World\n\n" );
}