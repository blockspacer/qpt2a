#include "TouchWay.h"

const Vec2f& TouchWay::TouchPoint::getPos() const { return m_pos; }
double TouchWay::TouchPoint::getTime() const { return m_dTime; }

TouchWay::TouchWay(): m_iId(0), m_way() {};

TouchWay::TouchWay(int32_t id, const Vec2f& startPos, double startTime):
	m_way(),
	m_iId(id)
{
	m_way.push_back(TouchPoint(startPos, startTime));
}

int32_t TouchWay::getId() const
{
	return m_iId;
}

Vec2f TouchWay::getStartPos() const
{
	return m_way.begin()->getPos();
}

Vec2f TouchWay::getCurrentPos() const
{
	return m_way.back().getPos();
}

void TouchWay::addPoint(const Vec2f& pos, double time)
{
	m_way.push_back(TouchPoint(pos, time));
}

Vec2f TouchWay::getLastDeltaVector() const
{
	if (m_way.size() < 2)
	{
		return Vec2f(0, 0); 
	}
	else
	{
		return (m_way.back().getPos() - m_way[m_way.size() - 2].getPos());
	}
}

bool TouchWay::isClick() const
{
	return (m_way.size() == 2 && getStartPos() == getCurrentPos());
}