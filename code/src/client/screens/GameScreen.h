#pragma once

#include "Screen.h"
#include "../rendering/Cam.h"
#include "../../common/Config.h"
#include "../../common/Player.h"
#include "../../common/GameObjectContainer.h"
#include "../../common/PolypeptideCapacityContainer.h"
#include "../actors/PolypeptideClient.h"
#include <unordered_map>
#include "../../common/network/messages/enum/CellType.h"
#include "boost/thread/mutex.hpp"
#include "../rendering/RenderManager.h"
#include <unordered_map>

class CellClient;
class GameObjectClient;
class GameScreenState;
class GameScreenStateNeutral;
class GameScreenStateInMenu;
class GameScreenStateCreateCell;
class GameScreenStateMovePolypeptides;
class GameScreenStateSelectPolypeptides;

class GameScreen :
	public Screen
{
public:
	friend class GameScreenState;
	friend class GameScreenStateNeutral;
	friend class GameScreenStateInMenu;
	friend class GameScreenStateCreateCell;
	friend class GameScreenStateGameOver;
	friend class GameScreenStateMovePolypeptides;
	friend class GameScreenStateSelectPolypeptides;

	class RenderText
	{
	public:
		RenderText(float timeDeath, ci::Vec3f pos, std::string text) :
			timeDeath(timeDeath),
			pos(pos),
			text(text)
		{}

		float getTimeDeath() const { return timeDeath; }
		ci::Vec3f getPos() const { return pos; }
		std::string getText() const { return text; }

	private:
		float timeDeath;
		ci::Vec3f pos;
		std::string text;
	};

	GameScreen();
	virtual ~GameScreen(void);

	virtual void update(float frameTime);
	virtual void draw();

	void setWorldRadius(float radius) { worldRadius = radius; }

	virtual bool touchBegan(const TouchWay & touchWay);
	virtual void touchMoved(const TouchWay & touchWay);
	virtual bool mouseMove(MouseEvent event);
	virtual void touchEnded(TouchWay touchWay);
	virtual bool touchClick(TouchWay touchWay);
	virtual void resize(ResizeEvent event);
	virtual void onKeyInput(KeyEvent& e);
	virtual void mouseWheel(MouseEvent & e);

	void addGameObjectToUpdate(GameObjectClient * gameObject);
	void removeGameObjectToUpdate(GameObjectClient * gameObject);

	void addGameObjectToDraw(GameObjectClient * gameObject);
	void removeGameObjectToDraw(GameObjectClient * gameObject);
	GameObjectContainer<GameObjectClient> & getGameObjectsToDraw();

	void addGameObjectToCollide(GameObject * gameObject);
	void removeGameObjectToCollide(GameObject * gameObject);

	void addCellToPick(CellClient * cell);
	void removeCellToPick(CellClient * cell);
	void pickCell(CellClient * cell);
	void unpickCells();
	CellClient * getPickedCell();

	void addIncompleteCell(CellClient * cell);
	void addIncompleteCell(
		unsigned int playerId, 
		CellType::Type type, 
		unsigned int cellId, 
		Vec3f position, 
		float angle
	);
	void removeIncompleteCell(CellClient * cell);
	void completeCellById(unsigned int id);

	void addCellPreview(CellClient * cell);
	void removeCellPreview(CellClient * cell);

	void addExploringCell(CellClient * cell);
	void removeExploringCell(CellClient * cell);
	GameObjectContainer<GameObject> & getExploringCells();

	IdGameObjectMap<PolypeptideClient> & getMyPolypeptides();

	IdGameObjectMap<PolypeptideClient> & getSelectedPolypeptides();

	bool canCellBePlaced(CellClient * cell);

	void addRenderText(RenderText const & text);
	ci::Vec2f worldToScreen(ci::Vec3f position);
	void switchToState(GameScreenState * newState);
	
	void updateVisibleGameObjects();
	void updateVisibilityOf(GameObjectClient * gameObject);

	void drawPolypeptideBar() const;

	boost::mutex & getContainerMutex();

	PolypeptideCapacityContainer * getPolypeptideCapacityContainer();

private:
	vector<CellClient *> getCellsPicked(Vec2f position);

	/// current state of the screen
	GameScreenState * state;
	/// the world's radius, i.e. the radius of the Petri's dish
	float worldRadius;
	/// cell currently picked
	CellClient * pickedCell;
	/// in-game menu for cell actions
	GUIItem * cellMenu;
	unordered_map<string, GUIItem *> cellMenuButtons;

	PolypeptideCapacityContainer polypeptideCapacityContainer;

	/// mutex for container manipulation and reading
	boost::mutex containerMutex;
	
	/// mutex for game state manipulation and reading
	boost::mutex switchScreenMutex;

	/// all game objects registered to be updated
	GameObjectContainer<GameObject>			gameObjectsToUpdate;

	/// all game objects registered to be drawn
	GameObjectContainer<GameObjectClient>	gameObjectsToDraw;

	/// all game objects registered to collide with each other
	GameObjectContainer<GameObject>			gameObjectsToCollide;

	/// all cells registered to be pickable
	GameObjectContainer<CellClient>			cellsToPick;
		
	/// all game objects that contribute to the visible part of the fog-of-war
	GameObjectContainer<GameObject>			cellsExploring;

	/// all areas that form the visible part of the fog-of-war
	GameObjectContainer<GameObject>			visibleAreas;

	/// all incomplete cells
	GameObjectContainer<CellClient>			cellsIncomplete;

	/// all cell previews
	std::set<CellClient *>					cellPreviews;

	/// my polypeptides
	IdGameObjectMap<PolypeptideClient>		myPolypeptides;

	/// selected polypeptides
	IdGameObjectMap<PolypeptideClient>		polypeptidesSelected;

	/// text which should be rendered to the screen
	std::vector<RenderText> textList;
};