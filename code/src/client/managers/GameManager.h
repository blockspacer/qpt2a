#pragma once

#include <stack>
#include "../../common/GameObject.h"
#include "../screens/Screen.h"
#include "../screens/GameScreen.h"
#include "ScreenManager.h"
#include "../network/ClientNetworkManager.h"
#include "../actors/PlayerClient.h"

#define GAME_MGR GameManager::getInstance()
#define GAME_SCR GAME_MGR->getScreenManager().getGameScreen()

class GameManager
{
public:
	~GameManager(void);

	static GameManager * const getInstance();

	void startGame(string playerName);
	void update(float frameTime);
	void draw();
	ScreenManager & getScreenManager();

	void addPetriDish(float radius);
	void addPlayer(unsigned int id, string name, unsigned int stemCellId, Vec3f stemCellPosition);
	void setMyPlayerId(unsigned int id);
private:
	/// singleton instance
	static GameManager* m_pManager;

	ScreenManager m_screenManager;

	/// id map for all active players
	map<unsigned int, PlayerClient*> players;
	/// my own player
	PlayerClient* myPlayer;

	boost::asio::ip::udp::endpoint serverEndpoint;
	ClientNetworkManager * networkManager;
	boost::thread networkManagerThread;

	/// game version
	string m_gameVersion;

	GameManager(void);
	GameManager(const GameManager&);
};

