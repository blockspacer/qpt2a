#pragma once
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <boost/thread/thread.hpp>
#include <vector>

#include "ConnectionEndpoint.h"
#include "messages/NetworkMessage.h"
#include "messages/ConnectionMessage.h"

class NetworkManager
{
public:
	NetworkManager(unsigned short listenPort);
	NetworkManager();
	virtual ~NetworkManager();
		
	void operator()();
	void send(NetworkMessage *message);
	
	template<typename T>
	void sendTo(T * message, std::vector<ConnectionEndpoint> endpoints)
	{
		unsigned size = message->calculateSize();

		char *copyData = new char[size];

		message->writeToArray(copyData);
		
		for (auto it = endpoints.begin(); it != endpoints.end(); ++it)
		{ 
			unsigned index = 0;
			T * copy = new T(copyData, index);
			copy->endpoint = (*it).m_endpoint;
			send(copy);
		}
		delete message;
	}

	void stop();


	
	virtual std::vector<ConnectionEndpoint> getConnectionEndpoints() = 0;
protected:
	virtual NetworkMessage* createNetworkMessage(char* data);
	virtual void handleMessage(NetworkMessage* message) = 0;
	virtual ConnectionEndpoint* getConnectionEndpoint(boost::asio::ip::udp::endpoint endpoint) = 0;

private:
	void handleConnectionMessage(ConnectionMessage* message);
	void baseSend(NetworkMessage &message);
	void connectionMaintenance();
	NetworkManager(const NetworkManager &other);

	boost::asio::ip::udp::socket *serverSocket;
	boost::asio::io_service *io_service;

	bool run;
	bool bound;
	boost::mutex maintenanceMutex;
	boost::thread maintenanceThread;

	bool networkThreadRunning;
	bool maintenanceThreadRunning;
	bool forceShutdown;
};