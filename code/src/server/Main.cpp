#define _USE_MATH_DEFINES

#include <iostream>
#include <boost/thread/thread.hpp>

#include "event/EventManager.h"

#include "game/GameContext.h"
#include "game/Game.h"

#include "../common/network/NetworkManager.h"

#include "../common/Config.h"

using namespace std;

int main(int argc, char argv[])
{
	try
	{
		int port = 2345;

		ConfigurationDataHandler::getInstance()->readFromXML("..\\..\\config.xml");

		Game game;

		GAMECONTEXT->initializeNetworkManager(port, &game);

		boost::thread networkThread(boost::bind(&NetworkManager::operator(), NETWORKMANAGER));
		boost::thread eventQueueThread(boost::bind(&EventManager::operator(), EVENT_MGR));

		networkThread.join();
		eventQueueThread.join();

		cout << "Application over";
		return 0;
	}
	catch(exception & e)
	{
		LOG_ERROR(e.what());
		return -1;
	}
	catch(string & e)
	{
		LOG_ERROR(e);
		return -1;
	}
}

