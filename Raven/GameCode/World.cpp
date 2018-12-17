#include "World.hpp"
#include "Utilities.hpp"

World::World(float width, float height)
	: movers(width, height), obstacles(width, height), walls(),
	width(width), height(height), cellWidth(width / partitionX), cellHeight(height / partitionY)
{
	walls.reserve(4);
}

std::vector<SGE::Object*> World::getObstacles(MovingObject* const mover, float radius)
{
	std::vector<SGE::Object*> obs;
	obs.reserve(5u);
	this->obstacles.CalculateNeighbours(obs, mover->getPosition(), radius);
	return obs;
}

std::vector<SGE::Object*> World::getObstacles(MovingObject* const mover)
{
	return this->getObstacles(mover, 10.f);
}

void World::getNeighbours(std::vector<MovingObject*>& res, MovingObject* const mover)
{
	return this->getNeighbours(res, mover, 10.f);
}

void World::getNeighbours(std::vector<MovingObject*>& res, MovingObject* const mover, float radius)
{
	this->getNeighbours(res, mover->getPosition(), radius);
	res.erase(std::remove(res.begin(), res.end(), mover), res.end());
}

void World::getNeighbours(std::vector<MovingObject*>& res, b2Vec2 position, float radius)
{
	this->movers.CalculateNeighbours(res, position, radius);
}

std::vector<std::pair<SGE::Object*, Wall>>& World::getWalls()
{
	return this->walls;
}

void World::AddObstacle(SGE::Object* ob)
{
	this->obstacles.AddEntity(ob);
}

void World::AddMover(MovingObject* mo)
{
	this->movers.AddEntity(mo);
}

void World::RemoveMover(MovingObject* mo)
{
	this->movers.RemoveEntity(mo);
}

void World::UpdateMover(MovingObject* mo, b2Vec2 oldPos)
{
	this->movers.UpdateEntity(mo, oldPos);
}

void World::AddWall(SGE::Object* wall, Wall::WallEdge edge)
{
	const b2Vec2 pos = wall->getPosition();
	const float w = wall->getShape()->getWidth();
	const float h = wall->getShape()->getHeight();
	b2Vec2 from, to;
	switch(edge)
	{
	case Wall::Left:
		from = b2Vec2{pos.x - 0.5f * w, pos.y - 0.5f * h};
		to = b2Vec2{pos.x - 0.5f * w, pos.y + 0.5f * h};
		break;
	case Wall::Right:
		from = b2Vec2{pos.x + 0.5f * w, pos.y + 0.5f * h};
		to = b2Vec2{pos.x + 0.5f * w, pos.y - 0.5f * h};
		break;
	case Wall::Top:
		from = b2Vec2{pos.x - 0.5f * w, pos.y + 0.5f * h};
		to = b2Vec2{pos.x + 0.5f * w, pos.y + 0.5f * h};
		break;
	case Wall::Bottom:
		from = b2Vec2{pos.x + 0.5f * w, pos.y - 0.5f * h};
		to = b2Vec2{pos.x - 0.5f * w, pos.y - 0.5f * h};
		break;
	default:;
	}
	this->walls.emplace_back(wall, Wall(from, to, edge));
}

void World::clear()
{
	this->movers.ClearCells();
	this->obstacles.ClearCells();
	this->walls.clear();
}

MovingObject* World::Raycast(b2Vec2 from, b2Vec2 direction, b2Vec2& hit) const
{
	for(size_t index : Ray(from, direction, this->width, this->height, this->cellWidth, this->cellHeight))
	{
		b2Vec2 moverHit, obstacleHit;
		MovingObject* hitMover = this->getHit(from, direction, moverHit, this->movers.getEntities(index));
		SGE::Object* hitObstacle = this->getHit(from, direction, obstacleHit, this->obstacles.getEntities(index));
		if(hitMover && hitObstacle)
		{
			if(b2DistanceSquared(from, moverHit) < b2DistanceSquared(from, obstacleHit))
			{
				hit = moverHit;
				return hitMover;
			}
			else
			{
				hit = obstacleHit;
				return nullptr;
			}
		}
		else if(hitMover)
		{
			hit = moverHit;
			return hitMover;
		}
		else if(hitObstacle)
		{
			hit = obstacleHit;
			return nullptr;
		}
	}
	float minDistance = std::numeric_limits<float>::max();
	float distance = minDistance;
	b2Vec2 tempHit;
	for(auto wall : this->walls)
	{
		LineIntersection(from, from + 1000.f * direction, wall.second.From(), wall.second.To(), distance, tempHit);
		if(distance < minDistance)
		{
			hit = tempHit;
			minDistance = distance;
		}
	}
	return nullptr;
}

World::Ray::RayIterator World::Ray::begin() const
{
	float tMaxX = 0.f, tMaxY = 0.f, tDeltaX = 0.f, tDeltaY = 0.f;
	const int deltaX = int(std::copysign(1.f, this->direction.x));
	const int deltaY = int(std::copysign(1.f, this->direction.y));
	int X = int(std::floor(this->from.x / this->cellWidth));
	int Y = int(std::floor(this->from.y / this->cellHeight));
	tMaxX = ((X + std::max(0, deltaX)) * this->cellWidth - this->from.x) / this->direction.x;
	tMaxY = ((Y + std::max(0, deltaY)) * this->cellHeight - this->from.y) / this->direction.y;
	tDeltaX = std::abs(this->cellWidth / this->direction.x);
	tDeltaY = std::abs(this->cellHeight / this->direction.y);
	return RayIterator(tMaxX, tMaxY, tDeltaX, tDeltaY, deltaX, deltaY, X, Y);
}

World::Ray::RayIterator World::Ray::end() const
{
	return RayIterator();
}

constexpr World::Ray::RayIterator::RayIterator(float max_x, float max_y, float delta_x, float delta_y,
											   int delta_x1, int delta_y1, int x, int y)
	:tDeltaX(delta_x), tDeltaY(delta_y), deltaX(delta_x1), deltaY(delta_y1),
	tMaxX(max_x), tMaxY(max_y), X(x), Y(y)
{}

World::Ray::Ray(b2Vec2 from, b2Vec2 direction, float width, float height, float cellWidth, float cellHeight)
	: from(from), direction(direction), width(width), height(height), cellWidth(cellWidth), cellHeight(cellHeight)
{}

World::Ray::RayIterator& World::Ray::RayIterator::operator++()
{
	if(tMaxX < tMaxY)
	{
		tMaxX += tDeltaX;
		X += deltaX;
	}
	else
	{
		tMaxY += tDeltaY;
		Y += deltaY;
	}
	return *this;
}

size_t World::Ray::RayIterator::operator*() const
{
	//std::cout << "X: " << this->X << " Y: " << this->Y << std::endl;
	return size_t(this->X + partitionX * Y);
}

bool World::Ray::RayIterator::operator==(const RayIterator& other) const
{
	return !this->operator!=(other);
}

bool World::Ray::RayIterator::operator!=(const RayIterator& other) const
{
	return (X >= 0 && X < partitionX) && (Y >= 0 && Y < partitionY);
}
