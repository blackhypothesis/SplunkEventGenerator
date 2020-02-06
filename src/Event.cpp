#include "Event.h"

EventGenerator::EventGenerator() :
		kB_s(10.0f), minWait_us(0), maxWait_us(0), minParam(10), maxParam(10), maxTryConnect(10), startTime_us(0), calculated_events_s(10), skip_events(20)
{
}

EventGenerator::EventGenerator(float inKB_s, int inMinParam, int inMaxParam) :
		kB_s(inKB_s), minParam(inMinParam), maxParam(inMaxParam), maxTryConnect(10)
{
	startTime_us = current_us();
	float calculated_kB_event = 0.023f + ((minParam + maxParam) / 2.0f * 0.013f);
	calculated_events_s = kB_s / calculated_kB_event;
	skip_events = static_cast<int>(calculated_events_s) * 2;
	int calculated_wait_us_per_event = 1000000 / calculated_events_s;
	minWait_us = static_cast<int>(calculated_wait_us_per_event * 0.9);
	maxWait_us = static_cast<int>(calculated_wait_us_per_event * 1.1);

	std::cout << currentDateTime() << " >>> INFO tid=" << std::this_thread::get_id() << " kB_s=" << kB_s << " minParam=" << minParam << " maxParam=" << maxParam << std::endl;
	std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " minWait=" << minWait_us << " maxWait=" << maxWait_us << std::endl;
}

long long unsigned int EventGenerator::current_us()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string EventGenerator::currentDateTime()
{
	std::string timeFormatString = "%F %X";
	auto currentTime = std::chrono::system_clock::now();
	std::time_t now = std::chrono::system_clock::to_time_t(currentTime);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch());
	auto secs = std::chrono::duration_cast<std::chrono::seconds>(ms);
	ms -= std::chrono::duration_cast<std::chrono::milliseconds>(secs);
	struct tm tstruct;
	char dateTime[80];
	tstruct = *localtime(&now);

	strftime(dateTime, sizeof(dateTime), timeFormatString.c_str(), &tstruct);

	std::stringstream dateTimeMilliSeconds;
	dateTimeMilliSeconds << std::string(dateTime) << "." << std::setfill('0') << std::setw(3) << ms.count();
	return dateTimeMilliSeconds.str();
}

std::string EventGenerator::generate(const unsigned int eventID, const unsigned int minParams,  unsigned int maxParams)
{
	const int HI = 100;
	const int LO = -100;
	std::vector<std::string> vecColor
	{ "RED", "YELLOW", "GREEN", "BLUE" };
	std::vector<std::string> vecPlanet
	{ "Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune", "Pluto" };


	const int color = static_cast<int>(rand()) / (static_cast<float>(RAND_MAX / (vecColor.size())));
	const int planet = static_cast<int>(rand()) / (static_cast<float>(RAND_MAX / (vecPlanet.size())));

	if (maxParams < minParams)
		maxParams = minParams;

	const int numberParams = minParams + static_cast<int>(rand()) / (static_cast<float>(RAND_MAX / (maxParams + 1 - minParams)));
	std::vector<float> vecFloat(numberParams);
	std::stringstream event;

	for (auto &v : vecFloat)
	{
		v = LO + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (HI - LO)));
	}

	event << currentDateTime() << " id=" << eventID << " ";
	event << "color=" << vecColor[color] << " planet=" << vecPlanet[planet] << " ";
	unsigned int paramNr = 0;

	for (auto &v : vecFloat)
	{
		paramNr++;
		event << "p" << std::to_string(paramNr) << "=\"" << v << "\" ";
	}
	event << "\n";

	return event.str();
}


std::string EventGenerator::generate(const unsigned int eventID, const std::string msg)
{
	std::stringstream event;
	event << currentDateTime() << " id=" << eventID << " " << msg << "\n";

	return event.str();
}


void EventGenerator::operator()()
{
	long long unsigned int totalBytes = 0;
	long long unsigned int start_create_event_us = 0;
	long long unsigned int time_create_event_us = 0;

	for (unsigned int eventID = 1; eventID >=1; eventID++)
	{
		if (maxWait_us > 0)
		{
			const int wait_us = minWait_us - time_create_event_us + static_cast<int>(rand()) / (static_cast<float>(RAND_MAX / (maxWait_us - minWait_us)));
			std::this_thread::sleep_for(std::chrono::microseconds(wait_us));
		}

		start_create_event_us = current_us();

		std::string event;

		if (eventID % skip_events)
		{
			event = generate(eventID, minParam, maxParam);
		}
		else
		{
			std::lock_guard<std::mutex> lock(mtx);
			auto deltaTime_us = current_us() - startTime_us;
			auto current_kB_s = static_cast<float>(totalBytes / 1024) / static_cast<float>(deltaTime_us / 1000000);
			auto pct_kB_s = current_kB_s / kB_s * 100;
			auto current_kB_event = static_cast<float>(totalBytes / 1024) / eventID;
			calculated_events_s = static_cast<float>(kB_s / current_kB_event);
			auto calculated_wait_us_per_event = 1000000 / calculated_events_s;
			minWait_us = static_cast<int>(calculated_wait_us_per_event * 0.9 * pct_kB_s / 100 * pct_kB_s / 100) - time_create_event_us;
			maxWait_us = static_cast<int>(calculated_wait_us_per_event * 1.1) - time_create_event_us;
			skip_events = 2000000 / calculated_wait_us_per_event;
			event = generate(eventID, "current_kB_s=" + std::to_string(current_kB_s) + " totalBytes=" + std::to_string(totalBytes) + " deltaTime_us=" + std::to_string(deltaTime_us) + " calculted_events_s=" + std::to_string(calculated_events_s));
		}

		std::cout << event;

		totalBytes += event.size();
		time_create_event_us = current_us() - start_create_event_us;
	}
}

int EventGenerator::operator()(std::string host, std::string port)
{
	using boost::asio::ip::tcp;
	std::string errorMsg;

	boost::system::error_code error;
	boost::asio::io_context io_context;
	tcp::resolver resolver(io_context);
	tcp::socket socket(io_context);
	int tryConnect = 0;
	long long unsigned int totalBytes = 0;

	std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " get endpoint for host=" << host << " port=" << port << std::endl;

	auto endpoints = resolver.resolve(host, port);

	try
	{
		boost::asio::connect(socket, endpoints);
	} catch (const boost::system::system_error &e)
	{
		std::lock_guard<std::mutex> lock(mtx);

		errorMsg = e.what();
		std::cout << currentDateTime() << " ERROR tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " tryConnect=" << tryConnect << " " << errorMsg << std::endl;

		tryConnect++;
	}

	std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " connection established" << std::endl;

	long long unsigned int start_create_event_us = 0;
	long long unsigned int time_create_event_us = 0;

	for (unsigned int eventID = 1; eventID >= 1; eventID++)
	{
		if (maxWait_us > 0)
		{
			const int wait_us = minWait_us - time_create_event_us + static_cast<int>(rand()) / (static_cast<float>(RAND_MAX / (maxWait_us - minWait_us)));
			std::this_thread::sleep_for(std::chrono::microseconds(wait_us));
		}

		start_create_event_us = current_us();

		std::string event;

		if (eventID % skip_events)
		{
			if (errorMsg.size() == 0)
				event = generate(eventID, minParam, maxParam);
			else
			{
				event = generate(eventID, errorMsg);
				errorMsg = "";
			}
		}
		else
		{
			std::lock_guard<std::mutex> lock(mtx);
			auto deltaTime_us = current_us() - startTime_us;
			auto current_kB_s = static_cast<float>(totalBytes / 1024) / static_cast<float>(deltaTime_us / 1000000);
			auto pct_kB_s = current_kB_s / kB_s * 100;
			auto current_kB_event = static_cast<float>(totalBytes / 1024) / eventID;
			calculated_events_s = static_cast<float>(kB_s / current_kB_event);
			auto calculated_wait_us_per_event = 1000000 / calculated_events_s;
			minWait_us = static_cast<int>(calculated_wait_us_per_event * 0.9 * pct_kB_s / 100 * pct_kB_s / 100) - time_create_event_us;
			maxWait_us = static_cast<int>(calculated_wait_us_per_event * 1.1) - time_create_event_us;
			skip_events = 2000000 / calculated_wait_us_per_event;
			std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " minWait=" << minWait_us << " maxWait=" << maxWait_us << std::endl;
			std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " current_kB_s=" << current_kB_s << std::endl;
			std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " pct_kB_s=" << pct_kB_s << "%" << std::endl;
			std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " current_kB_event=" << current_kB_event << std::endl;
			std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " calculated_events_s=" << calculated_events_s << std::endl;
			std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " calculated_wait_us_per_event=" << calculated_wait_us_per_event
					<< std::endl;
			std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " deltaTime_us=" << deltaTime_us << std::endl;
			std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " time_create_event_us=" << time_create_event_us << std::endl;
			std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " eventID=" << eventID << std::endl;
			std::cout << "-------------------------------------------------------------" << std::endl;

			event = generate(eventID, "totalBytes=" + std::to_string(totalBytes) + " deltaTime_us=" + std::to_string(deltaTime_us) + " calculted_events_s=" + std::to_string(calculated_events_s));
		}

		boost::asio::write(socket, boost::asio::buffer(event), error);

		if (error)
		{
			errorMsg = error.message();
			std::cout << currentDateTime() << " ERROR tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " " << errorMsg << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			try
			{
				boost::asio::connect(socket, endpoints);
				std::cout << currentDateTime() << " INFO tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " tryConnect=" << tryConnect << " connection established"
						<< std::endl;
			} catch (const boost::system::system_error &e)
			{
				std::lock_guard<std::mutex> lock(mtx);
				std::cout << currentDateTime() << " ERROR tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " tryConnect=" << tryConnect << " " << errorMsg << std::endl;
				errorMsg = e.what();
				tryConnect++;
				if (tryConnect > maxTryConnect)
				{
					std::cout << currentDateTime() << " ERROR tid=" << std::this_thread::get_id() << " host=" << host << " port=" << port << " tryConnect=" << tryConnect << " this was the last try "
							<< errorMsg << std::endl;
					return 1;
				}
			}
		}
		else
		{
			totalBytes += event.size();
		}

		time_create_event_us = current_us() - start_create_event_us;
	}
	return 0;
}

