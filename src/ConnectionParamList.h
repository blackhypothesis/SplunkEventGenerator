#pragma once

#include <iomanip>
#include <vector>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include "Event.h"

class ConnectionParamList
{
public:
	ConnectionParamList();

private:
	float kB_s;
	int minParam;
	int maxParam;
	long long unsigned int totalBytes;

	std::vector<std::vector<std::string>> vecDstHostPort;
	std::vector<boost::asio::ip::tcp::resolver::results_type> vecEndpoints;
	std::vector<long long unsigned int> vecTotalBytes;

public:
	void print() const;
	void start();
	std::vector<std::vector<std::string>> getVecDstHostPort() const;
};
