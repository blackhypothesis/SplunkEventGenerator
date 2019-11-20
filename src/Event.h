#pragma once
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <boost/asio.hpp>

#include <iostream>


class EventGenerator
{
public:
	EventGenerator();
	EventGenerator(float, int, int);

private:
	std::mutex mtx;
	float kB_s;
	int minWait_us;
	int maxWait_us;
	int minParam;
	int maxParam;
	int maxTryConnect;
	long long unsigned int startTime_us;
	float calculated_events_s;
	unsigned int skip_events;

	long long unsigned int current_us();
	std::string currentDateTime();
	std::string generate(const unsigned int, const unsigned int, unsigned int);
	std::string generate(const unsigned int, const std::string);

public:
	int operator()(std::string, std::string);
};



