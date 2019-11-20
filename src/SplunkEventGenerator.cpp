#include <thread>

#include "ConnectionParamList.h"

int main(int argc, char *argv[])
{
	ConnectionParamList cp;
	cp.print();
	cp.start();
	return 0;
}
