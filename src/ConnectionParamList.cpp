#include "ConnectionParamList.h"

ConnectionParamList::ConnectionParamList() :
	kB_s(10.0f),  minParam(10), maxParam(10), totalBytes(0)
{
	// initialize with the ENV variables DSTHOST, KB_S, MINPARAM, MAXPARAM
	//

	if (const char* envDstHost = std::getenv("DSTHOST"))
	{
		std::vector<std::string> vecAllHostPort;
		boost::split(vecAllHostPort, envDstHost, [](char c){return c == ';';});

		std::cout << "length " << vecAllHostPort.size() << std::endl;

		for (auto &hp : vecAllHostPort)
		{
			std::vector<std::string> vecHostPort;
			boost::split(vecHostPort, hp, [](char c){return c == ':';});

			if (vecHostPort.size() )
			vecDstHostPort.push_back(vecHostPort);
		}
	}
	else
	{
		std::cout << "ERROR env variable DSTHOST not defined. DSTHOST is necessary.\n";
		std::cout << "define DSTHOST=\"<host1>:<port1>;[<host2>:<port2>; ...]\"" << std::endl;
	}

	if (const char* envkb_s = std::getenv("KB_S"))
	{
		kB_s = static_cast<float>(std::atoi(envkb_s));
	}
	else
	{
		std::cout << "WARN env variable KB_S not defined. default value kB_s=" << kB_s << std::endl;
	}

	if (const char* envMinParam = std::getenv("MINPARAM"))
	{
		minParam = std::atoi(envMinParam);
	}
	else
	{
		std::cout << "WARN env variable MINPARAM not defined. default value minParam=" << minParam << std::endl;
	}

	if (const char* envMaxParam = std::getenv("MAXPARAM"))
	{
		maxParam = std::atoi(envMaxParam);
	}
	else
	{
		std::cout << "WARN env variable MAXPARAM not defined. default value maxParam=" << maxParam << std::endl;
	}

	totalBytes = 0;
}


void ConnectionParamList::print() const
{
	std::cout << "Parameters:\n";
	std::cout << "  kB_s=" << kB_s << "\n";
	std::cout << "  minParam=" << minParam << "\n";
	std::cout << "  maxParam=" << maxParam << "\n";

	for (auto &hp : vecDstHostPort)
	{
		std::cout << "host=" << hp[0] << " port=" << hp[1] << "\n";
	}

	std::cout << "\n" << std::endl;
}


void ConnectionParamList::start()
{
	float thread_kB_s = kB_s;
	if (vecDstHostPort.size() > 0)
		thread_kB_s = kB_s / vecDstHostPort.size();

	std::vector<std::thread> vecThread;
	EventGenerator eg(thread_kB_s, minParam, maxParam);

	// for each host:port create a EventGenerator thread
	for (auto hp : vecDstHostPort)
	{
		vecThread.push_back(std::thread(std::ref(eg), hp[0], hp[1]));
	}

	for (auto& t : vecThread)
		t.join();
}


std::vector<std::vector<std::string>> ConnectionParamList::getVecDstHostPort() const
{
	return vecDstHostPort;
}


