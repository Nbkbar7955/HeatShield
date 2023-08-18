// Columbia.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cstring>
#include <string>
#include <cstdio>
#include <utility>

using namespace std;

int main()
{
	pair<string, string> myTable;

	myTable.first = "This is myKey";
	myTable.second = "This is myValue";

	string myOut1 = "first -> "; // +myTable.first;
	string myOut2 = " second -> "; // +myTable.second;

	printf("%s\n%s\n", myOut1.c_str(),myOut2.c_str());


	cout << "\n -> ";
	cout << myTable.first;
	cout << "\n -> ";
	cout << myTable.second;
	cout << "\n";
    cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
