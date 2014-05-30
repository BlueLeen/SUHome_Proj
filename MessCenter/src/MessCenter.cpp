//============================================================================
// Name        : MessCenter.cpp
// Author      : leen
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <exception>
#include "MessCenterCall.h"
using namespace std;

int main() {
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	MessCenterCall* call = new MessCenterCall();
	call->Init();
	while(true)
	{
	}
	delete call;
	return 0;
}
