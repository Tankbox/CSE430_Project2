#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

bool readGraph(const char* graphFile)
{
	ifstream myFile(graphFile);
	int vertices;
	myFile >> vertices;
	myFile >> vertices;
	myFile.close();
	return false;
}

int main( int argc, const char* argv[] )
{
	readGraph(argv[1]);
	printf( "\nHello World\n" );
}