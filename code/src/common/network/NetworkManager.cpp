#include "NetworkManager.h"
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <iostream>

#include "messages/game/outgame/JoinRequest.h"
#include "messages/ConnectionMessage.h"

#include "../Config.h"

using boost::asio::ip::udp;
using namespace std;

NetworkManager::NetworkManager(unsigned short listenPort) : run(true), bound(true), networkThreadRunning(false), maintenanceThreadRunning(false), forceShutdown(false)
{
	io_service = new boost::asio::io_service();
	serverSocket = new boost::asio::ip::udp::socket(*io_service);
	serverSocket->open(udp::v4());
	serverSocket->set_option(boost::asio::ip::udp::socket::reuse_address(true));
	serverSocket->bind(udp::endpoint(udp::v4(), listenPort));
}

NetworkManager::NetworkManager() : run(true), bound(false), networkThreadRunning(false), maintenanceThreadRunning(false), forceShutdown(false)
{
	io_service = new boost::asio::io_service();
	serverSocket = new boost::asio::ip::udp::socket(*io_service);
	serverSocket->open(udp::v4());
    serverSocket->set_option(boost::asio::ip::udp::socket::reuse_address(true));
}

NetworkManager::NetworkManager(const NetworkManager &other) : run(true), bound(false), networkThreadRunning(false), maintenanceThreadRunning(false), forceShutdown(false)
{
	io_service = new boost::asio::io_service();
	serverSocket = new boost::asio::ip::udp::socket(*io_service, udp::endpoint(udp::v4(), other.serverSocket->local_endpoint().port()));
	serverSocket->set_option(boost::asio::ip::udp::socket::reuse_address(true));
	assert(false);
}

NetworkManager::~NetworkManager()
{
	forceShutdown = true;

	stop();

	if (serverSocket)
	{
		delete serverSocket;
	}
	
	if (io_service)
	{
		delete io_service;
	}
}
		
void NetworkManager::operator()()
{
	networkThreadRunning = true;
	maintenanceThread = boost::thread(boost::bind(&NetworkManager::connectionMaintenance, this));
	
	boost::array<char, 5000> buffer;
	try
	{
		while (run)
		{
			// On shut down of client.exe, sometimes the function body throws an access violation exception;
			// therefore the try-catch-block! Please have a look into this!
		
			udp::endpoint remote_endpoint;
			boost::system::error_code error;
		
			while (!bound && run);

			// Receive the message		
			serverSocket->receive_from(boost::asio::buffer(buffer), remote_endpoint, 0, error);
		
			maintenanceMutex.lock();
			if (!run || (error && error != boost::asio::error::message_size))
			{
				if (error && error != boost::system::errc::connection_reset && error != boost::system::errc::operation_would_block)
				{
					LOG_ERROR(std::string(error.category().name()) + ": " + error.message());
				}
				maintenanceMutex.unlock();
				continue;
			}
			// Distinguish the messages by their type
			NetworkMessage *message = createNetworkMessage(buffer.c_array());

			if (!message)
			{
				maintenanceMutex.unlock();
				continue;
			}
		
			message->endpoint = remote_endpoint;
		
			if (message && !dynamic_cast<ConnectionMessage*>(message))
			{
				ConnectionEndpoint *connectionEndpoint = getConnectionEndpoint(message->endpoint);
				if (connectionEndpoint)
				{
					for (unsigned i = connectionEndpoint->m_uiRemotePacketId; i < message->messageId; ++i)
					{
						/// Check whether the messageId is already missing and add it then
						if(std::find(connectionEndpoint->m_unreceivedMessages.begin(), connectionEndpoint->m_unreceivedMessages.end(), i) 
							== connectionEndpoint->m_unreceivedMessages.end()) 
						{
							connectionEndpoint->m_unreceivedMessages.push_back(i);
						}
					}
					connectionEndpoint->m_unreceivedMessages.remove(message->messageId);
					connectionEndpoint->m_uiRemotePacketId = max(message->messageId, connectionEndpoint->m_uiRemotePacketId + 1);
				}
			}
		
			// React to the Message and delete it.
			ConnectionMessage *connectionMessage = dynamic_cast<ConnectionMessage*> (message);
			if (connectionMessage)
			{
				handleConnectionMessage(connectionMessage);
				maintenanceMutex.unlock();
				delete message;
			}
			else
			{
				maintenanceMutex.unlock();

				ConnectionEndpoint *connectionEndpoint = getConnectionEndpoint(message->endpoint);
				if (connectionEndpoint)
				{
					/// Add the message
					connectionEndpoint->m_unhandledMessages.push(message);

					/// Find the lowest messageId of the missing messages
					auto currentMessageId = message->messageId;
					for (auto it = connectionEndpoint->m_unreceivedMessages.begin(); 
						it != connectionEndpoint->m_unreceivedMessages.end(); ++it)
					{
						if (*it < currentMessageId)
						{
							currentMessageId = *it;
						}
					}

					/// Handle all received messages until the next gap can be found
					while (!connectionEndpoint->m_unhandledMessages.empty() && 
						connectionEndpoint->m_unhandledMessages.top()->messageId == currentMessageId++)
					{
						NetworkMessage * currentMessage = connectionEndpoint->m_unhandledMessages.top();	
						connectionEndpoint->m_unhandledMessages.pop();
						handleMessage(currentMessage);
						delete currentMessage;
					}
				}
				else
				{
					handleMessage(message);
					delete message;
				}
			}
		}
	}
	catch(std::exception &ex)
	{
		std::cout << ex.what() << std::endl;
	}
	catch(std::string &ex)
	{
		std::cout << ex << std::endl;
	}
	catch(...)
	{
		// Don't know what else would be here
	}
	networkThreadRunning = false;
}

void NetworkManager::handleConnectionMessage(ConnectionMessage* message) {
	ConnectionEndpoint *connectionEndpoint = getConnectionEndpoint(message->endpoint);
	
	if (connectionEndpoint)
	{
		/// Resend all missing messages
		for (unsigned i = 0; i < message->missingMessageIds.size(); ++i)
		{
			if (connectionEndpoint->m_unconfirmedMessages.find(message->missingMessageIds[i]) != connectionEndpoint->m_unconfirmedMessages.end())
			{
				baseSend(*(connectionEndpoint->m_unconfirmedMessages[message->missingMessageIds[i]]));
			}
			else
			{
				// throw exception
				assert(false);
			}
		}

		std::vector<unsigned> toBeDeleted;

		/// Remove obsolete messages
		for(std::map<unsigned, NetworkMessage*>::iterator it = connectionEndpoint->m_unconfirmedMessages.begin(); it != connectionEndpoint->m_unconfirmedMessages.end(); ++it) 
		{
			if (it->first < message->messageId)
			{
				bool obsolete = true;
				for (unsigned i = 0; i < message->missingMessageIds.size(); ++i)
				{
					if (message->missingMessageIds[i] == it->first)
					{
						obsolete = false;
						break;
					}
				}

				if (obsolete)
				{
					toBeDeleted.push_back(it->first);
				}
			}
		}

		for (auto it = toBeDeleted.begin(); it != toBeDeleted.end(); ++it)
		{
			delete connectionEndpoint->m_unconfirmedMessages[*it];
			connectionEndpoint->m_unconfirmedMessages.erase(*it);
		}
	}
}

void NetworkManager::connectionMaintenance()
{
	try
	{
		maintenanceThreadRunning = true;
		while (run)
		{
			// On shut down of client.exe, sometimes the function body throws an access violation exception;
			// therefore the try-catch-block! Please have a look into this!
		
			boost::this_thread::sleep(boost::posix_time::milliseconds(100));
			maintenanceMutex.lock();
		
			if (!run) 
			{
				maintenanceMutex.unlock();
				continue;
			}

			std::vector<ConnectionEndpoint> endpoints =  getConnectionEndpoints();
			for (std::vector<ConnectionEndpoint>::iterator it = endpoints.begin(); it != endpoints.end(); ++it)
			{
				ConnectionMessage message;
				message.endpoint = (*it).m_endpoint;
				message.messageId = (*it).m_uiRemotePacketId;

				for (std::list<unsigned>::iterator idIt = (*it).m_unreceivedMessages.begin(); idIt != (*it).m_unreceivedMessages.end(); ++idIt) 
				{
					message.missingMessageIds.push_back(*idIt);
				}

				// Send the message
				baseSend(message);
			}
			maintenanceMutex.unlock();
		}
	}
	catch(std::exception &ex)
	{
		std::cout << ex.what() << std::endl;
	}
	catch(std::string &ex)
	{
		std::cout << ex << std::endl;
	}
	catch(...)
	{
		// Don't know what else would be here
	}
	maintenanceThreadRunning = false;
}

void NetworkManager::stop()
{
	run = false;
	try
	{
		serverSocket->shutdown(boost::asio::socket_base::shutdown_both);
		serverSocket->close();
	}
	catch(...)
	{
		/// Cannot do anything
	}

	float shutdownTime = 5.f;

	// Wait on threads (max 5 seconds)
	while ((maintenanceThreadRunning || networkThreadRunning) && !forceShutdown && shutdownTime > 0.f)
	{
		shutdownTime -= 0.1f;
		boost::this_thread::sleep(boost::posix_time::milliseconds(100));
	}
}

	
NetworkMessage* NetworkManager::createNetworkMessage(char* data)
{
	unsigned messageType;
	memcpy(&messageType, &data[sizeof(unsigned)], sizeof(unsigned));
	MessageType type(messageType);
		
	NetworkMessage *message = 0;

	unsigned index = 0;
	switch (type.getType())
	{
	case MessageType::ConnectionMessage:
		{
			message = new ConnectionMessage(data, index);
			break;
		}
	default:
		break;
	}

	return message;
}

void NetworkManager::baseSend(NetworkMessage &message)
{
	/// Set the size and create the send buffer
	message.messageSize = message.calculateSize();
	char* buffer = new char[message.messageSize];
	boost::system::error_code ignored_error;

	/// Serialize the Array, send the message and delete the buffer
	message.writeToArray(buffer);
	serverSocket->send_to(boost::asio::buffer(buffer, message.messageSize), message.endpoint, 0, ignored_error);
	delete[] buffer;
	bound = true;
}

void NetworkManager::send(NetworkMessage *message)
{
	/// Lock when changes in the connectionEndpoint are possible
	maintenanceMutex.lock();
	ConnectionEndpoint *connectionEndpoint = getConnectionEndpoint(message->endpoint);

	if (connectionEndpoint)
	{
		/// Increment the messageId and set it to identify the message
		message->messageId = connectionEndpoint->m_uiLocalPacketId++;
		connectionEndpoint->m_unconfirmedMessages[message->messageId] =  message;
	}

	baseSend(*message);
	
	maintenanceMutex.unlock();
}