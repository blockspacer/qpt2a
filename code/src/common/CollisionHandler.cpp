#include "CollisionHandler.h"

using namespace std;
using namespace ci;

CollisionHandler::CollisionHandler() : initialized(false) {}

CollisionHandler & CollisionHandler::initialize(Area const & screen)
{
	setScreenDimension(screen);
	clear();
	initialized = true;

	return *this;
}

CollisionHandler & CollisionHandler::clear()
{
	circleBackup.clear();
	buckets.clear();
	area.largestRadius = 0;
	area.bucketMeasures.x = area.dimensions.x2;
	area.bucketMeasures.y = area.dimensions.y2;
	initialized = false;
	backupInsert = false;

	return *this;
}

CollisionHandler & CollisionHandler::insert(Circle const & circle)
{
	Circle const * current = &circle;
	if (!backupInsert)
	{
		/// calculate grid size
		calculateBucketMeasures(*current);
	}

	set<unsigned int> bucketIndices = getBucketIndices(*current);
	for (auto it = bucketIndices.begin(); it != bucketIndices.end(); ++it)
	{
		if (!backupInsert)
		{
			current = &(circleBackup.insert(make_pair(current->index, *current)).first->second);
		}

		if (buckets.find(*it) == buckets.end())
		{
			buckets.insert(make_pair(*it, set<Circle const *>()));
		}
		buckets[*it].insert(current);
	}

	return *this;
}

set<unsigned int> CollisionHandler::getCircleIndicesToCheck(Circle const & circle) const
{
	set<unsigned int> circleIndices;
	set<unsigned int> bucketIndices = getBucketIndices(circle);
	for (auto itIndices = bucketIndices.begin(); itIndices != bucketIndices.end(); ++itIndices)
	{
		auto itBucket = buckets.find(*itIndices);
		if (itBucket != buckets.end())
		{
			for (auto itCircles = itBucket->second.begin();itCircles != itBucket->second.end(); ++itCircles)
			{
				circleIndices.insert((*itCircles)->index);
			}
		}
	}

	return circleIndices;
}

void CollisionHandler::setScreenDimension(Area const & screen)
{
	area.dimensions = screen;
}

void CollisionHandler::calculateBucketMeasures(Circle const & circle)
{
	if (area.largestRadius < circle.radius)
	{
		float & radius = area.largestRadius = circle.radius;
		float buffer = 1.f;

		float divisor = floor(area.dimensions.x2 / (2 * radius + buffer));
		area.bucketMeasures.x = area.dimensions.x2 / divisor;
		area.bucketMeasures.y = area.dimensions.y2 / (float)int(area.dimensions.y2 / (2 * radius + buffer));

		buckets.clear();

		backupInsert = true;
		for (auto it = circleBackup.begin(); it != circleBackup.end(); ++it)
		{
			insert(it->second);
		}
		backupInsert = false;
	}
}

unsigned int CollisionHandler::getBucketIndex(ci::Vec2f const & position) const
{
	int xpos = int(position.x / area.bucketMeasures.x);
	int ypos = int(position.y / area.bucketMeasures.y);
	int xs = int(area.dimensions.x2 / area.bucketMeasures.x);
	int ys = int(area.dimensions.y2 / area.bucketMeasures.y);
	return (ys - 1 - ypos) * xs + xpos;
}

set<unsigned int> CollisionHandler::getBucketIndices(Circle const & circle) const
{
	set<unsigned int> indices;
	unsigned int bucketIdx = getBucketIndex(circle.position);
	float p[][2] = {{-1.f, -1.f}, { 0.f, -1.f}, { 1.f, -1.f}, {-1.f, 0.f}, { 0.f, 0.f}, { 1.f, 0.f}, {-1.f, 1.f}, { 0.f, 1.f}, { 1.f, 1.f}};
	unsigned int p_length = 9;

	/// if it intersects with other buckets add it there also
	for (unsigned int i = 0; i < p_length; ++i)
	{
		Vec2f position = circle.position + Vec2f(p[i][0] * area.bucketMeasures.x, p[i][1] * area.bucketMeasures.y);
		if (isInScreen(position))
		{
			unsigned int curBucketIdx = getBucketIndex(position);
			Area & bucket = getBucketSizeFromIndex(curBucketIdx);

			if (aabbCollision.hasCollision(bucket, circle))
			{
				indices.insert(curBucketIdx);
			}
		}
	}
	return indices;
}

Area CollisionHandler::getBucketSizeFromIndex(unsigned int idx) const
{
	Area bucketSize;
	int nrOfColumns = int(area.dimensions.x2 / area.bucketMeasures.x);
	int nrOfRows = int(area.dimensions.y2 / area.bucketMeasures.y);
	int y = nrOfRows - 1 - (idx / nrOfColumns);
	int x = idx % nrOfColumns;

	bucketSize.x1 = x * area.bucketMeasures.x;
	bucketSize.y1 = y * area.bucketMeasures.y;
	bucketSize.x2 = bucketSize.x1 + area.bucketMeasures.x;
	bucketSize.y2 = bucketSize.y1 + area.bucketMeasures.y;

	return bucketSize;
}

bool CollisionHandler::isInScreen(ci::Vec2f const & position) const
{
	return position.x >= 0.f && 
			position.x <= area.dimensions.x2 && 
			position.y >= 0.f &&
			position.y <= area.dimensions.y2;
}