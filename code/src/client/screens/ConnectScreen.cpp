#include "ConnectScreen.h"
#include "../../common/network/NetworkManager.h"
#include "../managers/AssetManager.h"
#include "../managers/GameManager.h"
#include "TextInput.h"
#include "IpInput.h"
#include "../../common/ConfigurationDataHandler.h"

using namespace ci;

ConnectScreen::ConnectScreen()
{
	auto background = &(ASSET_MGR->getGuiTexture(string("soma-background")));
	rootItem->addSubItem(
		this,
		[]() {},
		Vec2f(CONFIG_FLOAT2("data.menu.connectcreen.background.x", 0), CONFIG_FLOAT2("data.menu.connectcreen.background.y", 0)),
		background,
		background,
		background
	);

	auto headline = &(ASSET_MGR->getGuiTexture(string("soma-connect-headline")));
	rootItem->addSubItem(
		this,
		[]() {},
		Vec2f(CONFIG_FLOAT2("data.menu.connectcreen.headline.x", 0), CONFIG_FLOAT2("data.menu.connectcreen.headline.y", 0)),
		headline,
		headline,
		headline
	);

	auto nameBox = &(ASSET_MGR->getGuiTexture(string("soma-connect-username-without-font")));

	TextInput *nameTextBox = new TextInput(
		this, 
		[]() { },
		Vec2f(CONFIG_FLOAT2("data.menu.connectcreen.username.x", 100), CONFIG_FLOAT2("data.menu.connectcreen.username.y", 40)), 
		nameBox, 
		nameBox, 
		nameBox
	);
	auto nameBoxItem = rootItem->addSubItem(nameTextBox);

	auto ipBox = &(ASSET_MGR->getGuiTexture(string("soma-connect-server-without-font")));
	auto ipBoxItem = rootItem->addSubItem(new IpInput(
		this,
		[]()
		{
			
		},
		Vec2f(CONFIG_FLOAT2("data.menu.connectcreen.server.x", 100), CONFIG_FLOAT2("data.menu.connectcreen.server.y", 150)),
		ipBox,
		ipBox,
		ipBox)
	);

	auto connectTextureBasic = &(ASSET_MGR->getGuiTexture(string("soma-connect-basic-connect")));
	auto connectTextureClick = &(ASSET_MGR->getGuiTexture(string("soma-connect-clicked-connect")));
	auto connectTextureHover = &(ASSET_MGR->getGuiTexture(string("soma-connect-hover-connect")));
	rootItem->addSubItem(
		this,
		[ipBoxItem, nameBoxItem]()
		{
			GAME_MGR->startGame(nameBoxItem->getValue(), ipBoxItem->getValue());
		},
		Vec2f(CONFIG_FLOAT2("data.menu.connectcreen.connect.x", 100), CONFIG_FLOAT2("data.menu.connectcreen.connect.y", 220)),
		connectTextureBasic,
		connectTextureClick,
		connectTextureHover
	);

	auto backBasic = &(ASSET_MGR->getGuiTexture(string("soma-connect-basic-back")));
	auto backClick = &(ASSET_MGR->getGuiTexture(string("soma-connect-clicked-back")));
	auto backHover = &(ASSET_MGR->getGuiTexture(string("soma-connect-hover-back")));
	rootItem->addSubItem(
		this,
		[]()
		{
			SCREEN_MGR->openMenuScreen();
		},
		Vec2f(CONFIG_FLOAT2("data.menu.connectcreen.back.x", 100), CONFIG_FLOAT2("data.menu.connectcreen.back.y", 300)),
		backBasic,
		backClick,
		backHover
	);
}

ConnectScreen::~ConnectScreen(void)
{
}

void ConnectScreen::update(float frameTime)
{
}

void ConnectScreen::draw()
{
	Screen::draw();
}

bool ConnectScreen::touchBegan(const TouchWay & touchWay)
{
	return Screen::touchBegan(touchWay);
	LOG_INFO("touch way started");
};

void ConnectScreen::touchMoved(const TouchWay & touchWay)
{
	LOG_INFO("touch way moved");
};

void ConnectScreen::touchEnded(TouchWay touchWay)
{
	Screen::touchEnded(touchWay);
	LOG_INFO("touch way ended");
};

void ConnectScreen::resize(ResizeEvent event)
{
}