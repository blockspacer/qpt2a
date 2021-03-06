#pragma once

#include "ConfigurationDataHandler.h"
#include "Cell.h"

class BoneCell :
	virtual public Cell
{
public:
	BoneCell(Vec3f position, float angle) :
		Cell(position, CONFIG_FLOAT("data.cell.bonecell.radius"), angle, CONFIG_FLOAT("data.cell.bonecell.healthpoints"))
	{ }
};
