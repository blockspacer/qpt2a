#include "EventCreator.h"

#include "../../common/Helper.h"

EventCreator* EventCreator::instance = 0;

EventCreator * EventCreator::getInstance()
{
	if (!instance)
	{
		instance = new EventCreator();
	}
	return instance;
}

void EventCreator::bind(NetworkManager * networkManager, EventQueue * eventQueue, GameObjectContainer * gameObjectContainer, vector<PlayerServer *> * players)
{
	this->networkManager = networkManager;
	this->eventQueue = eventQueue;
	this->gameObjectContainer = gameObjectContainer;
	this->players = players;
}

bool EventCreator::createBuildEvent(const double time, const unsigned int requestId, const int type, const float angle, PlayerServer & currentPlayer, CellServer & cell)
{
	if (!isInRadiusOf<float>(cell.getPosition(), cell.getRadius(), Vec3f::zero(), CONFIG_FLOAT1("data.world.radius")))
	{
		CreateCellFailure *failure = new CreateCellFailure();
		failure->requestId = requestId;
		failure->errorCode = CreateCellErrorCode::OutsideGameArea;
		networkManager->send(failure);
		return false;
	}

	auto it = players->begin();
	for (; it != players->end(); ++it)
	{
		const vector<GameObject *> & gameObjects = gameObjectContainer->findInRadiusOf(cell.getPosition(), cell.getRadius());
		if (gameObjects.size() > 0)
		{
			/// collision detected
			CreateCellFailure *failure = new CreateCellFailure();
			failure->requestId = requestId;
			failure->errorCode = CreateCellErrorCode::SpotAlreadyTaken;
			networkManager->send(failure);
			return false;
		}
	}

	gameObjectContainer->createGameObject(&cell);

	BuildingEvent * be = new BuildingEvent(time, *networkManager, cell, *players);
	eventQueue->addEvent(be);
		
	CreateCellSuccess *success = new CreateCellSuccess();
	success->requestId = requestId;
	success->cellId = cell.getId();
	success->position[0] = cell.getPosition()[0];
	success->position[1] = cell.getPosition()[1];
	success->angle = cell.getAngle();
	networkManager->send(success);

	for (unsigned int i = 0; i < players->size(); ++i)
	{
		if ((*players)[i]->getId() != currentPlayer.getId())
		{
			CellNew cellNew;
			cellNew.playerId = currentPlayer.getId();
			cellNew.cellId = cell.getId();
			cellNew.position[0] = cell.getPosition()[0];
			cellNew.position[1] = cell.getPosition()[1];
			cellNew.type = type;
			cellNew.endpoint = (*players)[i]->getEndpoint();
		}
	}

	return true;
}

bool EventCreator::createAttackEvent(const double time, const bool isAttacker, const PlayerServer & currentPlayer, CellServer & currentCell)
{
	if (!(networkManager && eventQueue && players))
	{
		/// call bind first
		return false;
	}

	if (isAttacker && (!currentCell.getIsComplete() || dynamic_cast<StandardCellServer *>(&currentCell) == 0))
	{
		/// cell could not be an attacker
		return false;
	}

	auto playersIt = players->begin();
	for (; playersIt != players->end(); ++playersIt)
	{
		if ((*playersIt)->getId() != currentPlayer.getId())
		{
			/** 
				* search for all cells that would be in its attack radius
				* reversely all the other cell that are attackers have the current cell in its attack radius
				*/
			const vector<GameObject *> & gameObjects =
				gameObjectContainer->findInRadiusOf(currentCell.getPosition(), currentCell.getRadius() + CONFIG_FLOAT1("data.cell.standardcell.attackradius"));
					
			ci::Vec3f attacker;
			ci::Vec3f victim;
			/// those assignments are true for all iterations
			if (isAttacker)
			{
				attacker = ci::Vec3f(currentCell.getPosition());
			}
			else
			{
				ci::Vec3f victim(currentCell.getPosition());
			}
			for (auto it = gameObjects.begin(); it != gameObjects.end(); ++it)
			{
				CellServer * actualCell = dynamic_cast<CellServer *>(*it);
				if (actualCell != 0 && actualCell->getIsComplete())
				{
					/// those assignments depend on the iterator
					if (isAttacker)
					{
						victim = ci::Vec3f(actualCell->getPosition());
					}
					else
					{
						if (actualCell->getIsComplete() && dynamic_cast<StandardCellServer*>(actualCell) == 0) continue;
						ci::Vec3f attacker(actualCell->getPosition());
					}

					ci::Vec3f attackerDir(Vec3f::xAxis());
					attackerDir.rotate(Vec3f::zAxis(), actualCell->getAngle() * (float)M_PI / 180.f);
					attackerDir.normalize();

					ci::Vec3f attacker2VictimDir;
					attacker2VictimDir = victim - attacker;
					attacker2VictimDir.normalize();

					float modifier = attackerDir.dot(attacker2VictimDir);
					float distanceDropOffDegree = CONFIG_FLOAT1("data.cell.standardcell.distanceDropOffDegree");
					float cosOfDDOD = cosf(distanceDropOffDegree * (float)M_PI / 180.f);
					if (modifier >= cosOfDDOD)
					{
						modifier -= cosOfDDOD;
						modifier /= (1.f + cosOfDDOD);

						float damage = CONFIG_FLOAT1("data.cell.standardcell.damage") * modifier;

						CellServer * attackerCell = isAttacker ? &currentCell : actualCell;
						CellServer * victimCell = isAttacker ? actualCell : &currentCell;
						AttackEvent * ae = new AttackEvent(time, *networkManager, *eventQueue, *gameObjectContainer, *attackerCell, *victimCell, damage, *players);
					}

					/// attack message is sent in event
				}
			}
		}
	}
	return true;
}

EventCreator::EventCreator() { }