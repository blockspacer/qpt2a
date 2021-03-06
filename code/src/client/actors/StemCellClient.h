#pragma once
#include "CellClient.h"
#include "../../common/StemCell.h"
#include "../actors/PlayerClient.h"

class StemCellClient :
 virtual public StemCell,
 virtual public CellClient
{
public:
 StemCellClient(unsigned int id, Vec3f position, float angle, PlayerClient * owner) :
	Cell(position, CONFIG_FLOAT("data.cell.stemcell.radius"), angle, CONFIG_FLOAT("data.cell.stemcell.healthpoints"), owner),
	CellClient(),
	StemCell(position, angle)
{
	this->type = CellType::StemCell;
	setId(id);
	this->polyMax = CONFIG_INT("data.polypeptide.maxPerStemCell");
}
 
	virtual void update(float frameTime) { CellClient::update(frameTime); }
	virtual void setPosition(Vec3f position) { CellClient::setPosition(position); }

	virtual bool addPolypeptide(Polypeptide * polypeptide) { return CellClient::addPolypeptide(polypeptide); }
 
private:
	virtual void setId(unsigned int id) { CellClient::setId(id); }
	virtual void setRotation(Vec3f rotation) { CellClient::setRotation(rotation); }
	virtual void setScale(Vec3f scale) { CellClient::setScale(scale); }
	virtual void setRadius(float radius) { CellClient::setRadius(radius); }
	virtual void drawAtTransformation() const;
};