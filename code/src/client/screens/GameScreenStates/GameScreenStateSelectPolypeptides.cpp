#include "GameScreenStateSelectPolypeptides.h"
#include "GameScreenStateMovePolypeptides.h"
#include "GameScreenStateInMenu.h"
#include "../../managers/GameManager.h"

GameScreenStateSelectPolypeptides::GameScreenStateSelectPolypeptides(GameScreen * screen, CellClient * pickedCell):
	GameScreenState(screen),
	hasDragged(false),
	maxPickRadius(CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.maxPickRadius")),
	pickedCell(pickedCell),
	numberOfPolypeptidesSelected(0),
	pickRadius(0)
{
	polypeptideCount = pickedCell->getPolypeptides().size();
	onePolypeptideDistance = maxPickRadius / float(polypeptideCount);
	center = pickedCell->getPosition2D();
}

bool GameScreenStateSelectPolypeptides::touchClick(TouchWay touchWay)
{
	screen->switchToState(new GameScreenStateInMenu(screen, pickedCell));
	return true;
}

bool GameScreenStateSelectPolypeptides::touchBegan(const TouchWay & touchWay)
{
	return false;
}

void GameScreenStateSelectPolypeptides::touchMoved(const TouchWay & touchWay)
{
	if (!touchWay.isClick())
	{
		hasDragged = true;
	}

	pickRadius = touchWay.getCurrentPos().distance(pickedCell->getPosition2D());
	pickRadius = min(maxPickRadius, pickRadius);

	numberOfPolypeptidesSelected = ceil(pickRadius / onePolypeptideDistance);
}

void GameScreenStateSelectPolypeptides::touchEnded(const TouchWay & touchWay)
{
	GAME_SCR->getContainerMutex().lock();
	auto it = pickedCell->getPolypeptides().begin();
	for (unsigned int i = 0; i < numberOfPolypeptidesSelected && it != pickedCell->getPolypeptides().end(); ++i, ++it)
	{
		screen->getSelectedPolypeptides().addGameObject(dynamic_cast<PolypeptideClient* >(it->second));
	}
	GAME_SCR->getContainerMutex().unlock();

	screen->switchToState(new GameScreenStateMovePolypeptides(screen));
}

void GameScreenStateSelectPolypeptides::draw2D()
{
	if (hasDragged)
	{
		gl::color(ColorA(
			CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.stepRadiusColor.r"),
			CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.stepRadiusColor.g"),
			CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.stepRadiusColor.b"),
			CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.stepRadiusColor.a")
		));

		for (unsigned int i = 1; i < polypeptideCount; ++i)
		{
			gl::drawStrokedCircle(center, i * onePolypeptideDistance);
		}

		gl::color(ColorA(
			CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.maxRadiusColor.r"),
			CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.maxRadiusColor.g"),
			CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.maxRadiusColor.b"),
			CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.maxRadiusColor.a")
		));
		gl::drawStrokedCircle(pickedCell->getPosition2D(), maxPickRadius);

		gl::drawString(stringify(ostringstream() << numberOfPolypeptidesSelected), center, ColorA(1.f, 1.f, 1.f, 1.f), 
			Font(CONFIG_STRING("data.ingamefeedback.renderedDamage.font"), (float) CONFIG_INT("data.ingamefeedback.renderedDamage.size")));
	
		gl::color(ColorA(
			CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.selectRadiusColor.r"),
			CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.selectRadiusColor.g"),
			CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.selectRadiusColor.b"),
			CONFIG_FLOAT("data.ingamefeedback.pickPolypeptides.selectRadiusColor.a")
		));
		gl::drawSolidCircle(center, numberOfPolypeptidesSelected * onePolypeptideDistance);
	}
}