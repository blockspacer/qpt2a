#pragma once

class CellServer;
class PlayerServer;

#include <vector>
#include <map>

#include "GameEvent.h"


class BuildingEvent : public GameEvent
{
public:
	BuildingEvent(double startTime, unsigned int cellId);

	virtual void trigger();
private:
	unsigned int cellId;
};