#include "PolypeptideFightEvent.h"
#include "EventManager.h"
#include "../game/CellServer.h"
#include "../game/PlayerServer.h"
#include "../game/GameContext.h"
#include "../../common/GameObjectContainer.h"
#include "../../common/network/NetworkManager.h"
#include "../../common/network/messages/game/ingame/polypeptide/combat/PolypeptideFight.h"
#include "../../common/network/messages/game/ingame/polypeptide/combat/PolypeptideDie.h"
#include "../../common/PolypeptideCapacityContainer.h"
#include <cinder/Rand.h>

PolypeptideFightEvent::PolypeptideFightEvent(double startTime, unsigned int cellId1, unsigned int cellId2, unsigned int polypeptideId1, unsigned int polypeptideId2) :
	cellId1(cellId1),
	cellId2(cellId2),
	polypeptideId1(polypeptideId1),
	polypeptideId2(polypeptideId2),
	GameEvent(startTime, CONFIG_FLOAT("data.event.polypeptidefight.time"))
{ }

void PolypeptideFightEvent::trigger()
{
	auto cell1 = dynamic_cast<CellServer *>(GAMECONTEXT->getActiveCells().find(cellId1));
	auto cell2 = dynamic_cast<CellServer *>(GAMECONTEXT->getActiveCells().find(cellId2));
	if (cell1 != nullptr && cell2 != nullptr)
	{
		auto polypeptide1 = cell1->getPolypeptides().find(polypeptideId1)->second;
		auto polypeptide2 = cell2->getPolypeptides().find(polypeptideId2)->second;
		if (polypeptide1 != nullptr && polypeptide2 != nullptr &&
			(polypeptide1->getState() & polypeptide2->getState()) == Polypeptide::POLYPEPTIDEFIGHT)
		{
			bool polypeptide1Dies = ci::randBool();
			bool polypeptide2Dies = ci::randBool();
			PolypeptideFight * polypeptideFight = new PolypeptideFight();
			polypeptideFight->polypeptideId1 = polypeptideId1;
			polypeptideFight->polypeptideId2 = polypeptideId2;
			polypeptideFight->polypeptide1Dies = polypeptide1Dies;
			polypeptideFight->polypeptide2Dies = polypeptide2Dies;
			NETWORKMANAGER->sendTo<PolypeptideFight>(polypeptideFight, NETWORKMANAGER->getConnectionEndpoints());

			if (!polypeptide1Dies && polypeptide2Dies)
			{
				polypeptide1->setState(Polypeptide::IDLE);
			}
			else if (polypeptide1Dies && !polypeptide2Dies)
			{
				polypeptide2->setState(Polypeptide::IDLE);
			}
			else if (!polypeptide1Dies && !polypeptide2Dies)
			{
				(*EVENT_MGR) += new PolypeptideFightEvent(this->m_dDeadTime, cellId1, cellId2, polypeptideId1, polypeptideId2);
			}

			if (polypeptide1Dies)
			{
				PolypeptideDie * polypeptideDie = new PolypeptideDie();
				polypeptideDie->polypeptideId = polypeptideId1;
				NETWORKMANAGER->sendTo<PolypeptideDie>(polypeptideDie, NETWORKMANAGER->getConnectionEndpoints());

				cell1->removePolypeptide(polypeptide1);
				delete polypeptide1;
				--(POLYCAPACITY->NumberOfPolypeptides);
				GAMECONTEXT->getAttackRelations().update();
			}

			if (polypeptide2Dies)
			{
				PolypeptideDie * polypeptideDie = new PolypeptideDie();
				polypeptideDie->polypeptideId = polypeptideId2;
				NETWORKMANAGER->sendTo<PolypeptideDie>(polypeptideDie, NETWORKMANAGER->getConnectionEndpoints());

				cell2->removePolypeptide(polypeptide2);
				delete polypeptide2;
				--(POLYCAPACITY->NumberOfPolypeptides);
				GAMECONTEXT->getAttackRelations().update();
			}
		}
	}
}