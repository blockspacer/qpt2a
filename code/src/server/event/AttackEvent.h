#pragma once

class NetworkManager;
class CellServer;

#include <vector>

#include "../../common/GameObjectContainer.h"
#include "GameEvent.h"

class AttackEvent : public GameEvent
{
public:
	AttackEvent(double startTime, NetworkManager & manager, CellServer & attacker, CellServer & victim, float damage);

	virtual void trigger();
private:
	NetworkManager & manager;
	CellServer & attacker;
	CellServer & victim;
	float damage;
};