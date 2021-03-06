#include "BuildingEvent.h"

#include "../../common/Config.h"
#include "../../common/ConfigurationDataHandler.h"

#include "../../common/network/NetworkManager.h"
#include "../../common/network/messages/game/ingame/cell/creation/CreateCellComplete.h"

#include "../game/CellServer.h"
#include "../game/PlayerServer.h"
#include "../game/GameContext.h"

#include "../environment/StaticModificatorServer.h"

#include "cinder/Rand.h"

#include "EventCreator.h"

BuildingEvent::BuildingEvent(double startTime, unsigned int cellId) :
	cellId(cellId)
	{ 
		double duration = CONFIG_FLOAT("data.event.build.standardcell.time");

		auto & cell = *(dynamic_cast<CellServer *>(GAMECONTEXT->getInactiveCells().find(cellId)));
		if (cell.getType() == CellServer::BONECELL)
		{
			duration = CONFIG_FLOAT("data.event.build.bonecell.time");
		}

		auto modifiers = cell.getStaticModificator();
		for (auto it = modifiers.begin(); it != modifiers.end(); ++it)
		{
			switch ((*it)->getType())
			{
			case StaticModificator::RADIOACTIVITY:
				duration *= (1 + ci::randFloat());
				break;

			case StaticModificator::NUTRIENTSOIL:
				duration /= (1 + ci::randFloat());
				break;
			default:
				break;
			}
		}

		this->m_dDeadTime = startTime + duration;
	}

void BuildingEvent::trigger()
{
	auto cellO = GAMECONTEXT->getInactiveCells().find(cellId);
	if (cellO != nullptr)
	{
		auto & cell = *(dynamic_cast<CellServer *>(cellO));
		cell.completeCell();

		auto obj = GAMECONTEXT->getInactiveCells().find(cell.getId());
		if (obj == nullptr)
		{
			LOG_INFO("cell not in list");
			return;
		}
		GAMECONTEXT->getActiveCells().createGameObject(obj);
		GAMECONTEXT->getInactiveCells().removeGameObject(cell.getId(), false);

		LOG_INFO(stringify(ostringstream() << "Cell with the id " << cell.getId() << " is finished"));

		PlayerServer * current = GAMECONTEXT->getPlayer(cell.getOwner()->getId());

		CreateCellComplete * complete = new CreateCellComplete();
		complete->cellId = cell.getId();

		NETWORKMANAGER->sendTo<CreateCellComplete>(complete, NETWORKMANAGER->getConnectionEndpoints());
		LOG_INFO("CreateCellComplete sent");

		EVENT_CRTR->createAttackEvent(m_dDeadTime, true, cell);
		EVENT_CRTR->createAttackEvent(m_dDeadTime, false, cell);
	}
}