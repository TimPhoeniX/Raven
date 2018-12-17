#pragma once
#include "CellSpacePartition.hpp"
#include "MovingObject.hpp"
#include "Wall.hpp"
#include "Utilities.hpp"

namespace
{
	constexpr size_t partitionX = 8u;
	constexpr size_t partitionY = 8u;
}

class World
{
protected:
	CellSpacePartition<MovingObject, partitionX, partitionY> movers;
	CellSpacePartition<SGE::Object, partitionX, partitionY> obstacles;
	std::vector<std::pair<SGE::Object*, Wall>> walls;
	const float width, height, cellWidth, cellHeight;
public:
	class Ray
	{
	protected:
		b2Vec2 from, direction;
		float width, height, cellWidth, cellHeight;
	public:
		Ray(b2Vec2 from, b2Vec2 direction, float width, float height, float cellWidth, float cellHeight);
		class RayIterator
		{
			const float tDeltaX = 0.f , tDeltaY = 0.f;
			const int deltaX = 0u, deltaY = 0u;
			float tMaxX = 0.f, tMaxY = 0.f;
			int X = 0u, Y = 0u;
		public:
			constexpr RayIterator() = default;
			constexpr RayIterator(float max_x, float max_y, float delta_x, float delta_y,
								  int delta_x1, int delta_y1, int x, int y);

			RayIterator& operator++();
			size_t operator*() const;
			bool operator==(const RayIterator& other) const;
			bool operator!=(const RayIterator& other) const;
		};
		RayIterator begin() const;
		RayIterator end() const;
	};
	World(float width, float height);

	std::vector<SGE::Object*> getObstacles(MovingObject* const mover, float radius);

	std::vector<SGE::Object*> getObstacles(MovingObject* const mover);

	void getNeighbours(std::vector<MovingObject*>& res, MovingObject* const mover);

	void getNeighbours(std::vector<MovingObject*>& res, MovingObject* const mover, float radius);

	void getNeighbours(std::vector<MovingObject*>& res, b2Vec2 position, float radius);

	std::vector<std::pair<SGE::Object*, Wall>>& getWalls();

	void AddObstacle(SGE::Object* ob);

	void AddMover(MovingObject* mo);

	void UpdateMover(MovingObject* mo, b2Vec2 oldPos);

	void AddWall(SGE::Object* wall, Wall::WallEdge edge);

	void clear();

	template<typename T>
	T* getHit(b2Vec2 from, b2Vec2 direction, b2Vec2& hit, const std::vector<T*>& entities) const
	{
		T* closestObject = nullptr;
		float closestDist = std::numeric_limits<float>::max();
		for(T* ob : entities)
		{
			b2Vec2 localPos = PointToLocalSpace(ob->getPosition(), direction, from);
			float obRadius = ob->getShape()->getRadius();
			if(localPos.x >= 0.f)
			{
				if(b2Abs(localPos.y) < obRadius)
				{
					float cX = localPos.x, cY = localPos.y;
					float sqrtPart = sqrt(obRadius*obRadius - cY*cY);
					float ip = cX - sqrtPart;
					if(ip <= 0.f) ip = cX + sqrtPart;
					if(ip < closestDist)
					{
						closestDist = ip;
						closestObject = ob;
						hit = PointToWorldSpace(b2Vec2{ip,0.f}, direction, from);
					}
				}
			}
		}
		return closestObject;
	}

	MovingObject* Raycast(b2Vec2 from, b2Vec2 direction, b2Vec2& hit) const;
	void RemoveMover(MovingObject* hitObject);
};
