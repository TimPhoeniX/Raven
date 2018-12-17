#pragma once
#include "Box2D/Common/b2Math.h"
#include <vector>
#include <array>
#include <set>

namespace
{
	template<typename Vec, typename T>
	void EraMove(Vec& v, const T& e)
	{
		v.erase(std::remove(v.begin(), v.end(), e), v.end());
	}
}

struct AABB
{
	b2Vec2 low = b2Vec2_zero;
	b2Vec2 high = b2Vec2_zero;
	inline bool isOverlapping(const AABB& other) const
	{
		return !((other.high.y < this->low.y)
				 || (other.low.y > this->high.y)
				 || (other.low.x > this->high.x)
				 || (other.high.x < this->low.x));
	}
	inline b2Vec2 UpperLeft() const
	{
		return {this->low.x, this->high.y};
	}
	inline b2Vec2 UpperRight() const
	{
		return this->high;
	}
	inline b2Vec2 LowerLeft() const
	{
		return this->low;
	}
	inline b2Vec2 LowerRight() const
	{
		return {this->high.x, this->low.y};
	}
	AABB() = default;
	inline AABB(b2Vec2 low, b2Vec2 high): low(low), high(high){}
};

template<typename T>
struct Cell
{
	std::vector<T*> Entities;
	AABB aabb;
	Cell()
	{
		this->Entities.reserve(10u);
	}
	Cell(const Cell&) = default;
	Cell(Cell&&) = default;
	Cell(b2Vec2 low, b2Vec2 high): aabb{low, high}
	{}
	explicit Cell(AABB aabb): aabb(aabb)
	{}
};

template<typename T, size_t X, size_t Y>
class CellSpacePartition
{
	class AABBQuery
	{
		size_t beginX, beginY, endX, endY;
	public:
		explicit AABBQuery(const CellSpacePartition*const space, const AABB& query)
		{
			b2Vec2 low = b2Clamp(query.low, b2Vec2_zero, {space->width,space->height});
			b2Vec2 high = b2Clamp(query.high, b2Vec2_zero, {space->width,space->height});
			beginX = size_t(X * low.x / space->width);
			beginX = beginX < X ? beginX : X - 1;
			beginY = size_t(Y * low.y / space->height);
			beginY = beginY < Y ? beginY : Y - 1;
			endX = size_t(X * high.x / space->width);
			endX = endX < X ? endX : X - 1;
			endY = size_t(Y * high.y / space->height);
			endY = endY < Y ? endY : Y - 1;
		}
		class iterator: std::iterator<std::input_iterator_tag, size_t>
		{
			const AABBQuery* owner;
			size_t x, y;
		public:
			iterator(size_t x, size_t y, const AABBQuery* owner): owner(owner), x(x), y(y)
			{}
			bool operator==(const iterator& other) const
			{
				return this->x == other.x && this->y == other.y;
			}
			bool operator!=(const iterator& other) const
			{
				return !this->operator==(other);
			}
			iterator& operator++()
			{
				if(this->x == this->owner->endX)
				{
					this->x = this->owner->beginX;
					++this->y;
				}
				else
				{
					++this->x;
				}
				return *this;
			}
			size_t operator*() const
			{
				return size_t(x + X*y);
			}
		};
		iterator begin()
		{
			return iterator(beginX, beginY, this);
		}
		iterator end()
		{
			return iterator(beginX, endY+1u, this);
		}
	};
	std::array<Cell<T>, X*Y> cells;
	const float width;
	const float height;
	const float cellWidth;
	const float cellHeight;
public:
	CellSpacePartition(float width, float height): width(width), height(height), cellWidth(width / X), cellHeight(height / Y)
	{
		for(size_t y = 0u; y < Y; ++y)
		{
			for(size_t x = 0u; x < X; ++x)
			{
				b2Vec2 low{x * this->cellWidth, y * this->cellHeight};
				b2Vec2 high{low.x + this->cellWidth, low.y + this->cellHeight};
				AABB aabb = AABB(low, high);
				this->cells[x + X*y].aabb = aabb;
			}
		}
	}

	void ClearCells()
	{
		for(Cell<T>& c : cells)
		{
			c.Entities.clear();
		}
	}

	void CalculateNeighbours(std::vector<T*>& res, b2Vec2 pos, const float radius)
	{
		AABB query{pos - b2Vec2{radius,radius}, pos + b2Vec2{radius, radius}};
		/*for(Cell<T>& cell : this->cells)
		{
			if(!cell.Entities.empty() && cell.aabb.isOverlapping(query))
			{
				for(T* en : cell.Entities)
				{
					if(b2DistanceSquared(en->getPosition(), pos) < radius*radius)
					{
						res.push_back(en);
					}
				}
			}
		}*/
		std::set<T*> setOfNeighbours;
		AABBQuery list(this, query);
		for(size_t it: list)
		{
			auto& cell = this->cells[it];
			if(!cell.Entities.empty() && cell.aabb.isOverlapping(query))
			{
				for(T* en : cell.Entities)
				{
					if(b2DistanceSquared(en->getPosition(), pos) < radius*radius)
					{
						setOfNeighbours.insert(en);
					}
				}
			}
		}
		res.assign(setOfNeighbours.begin(), setOfNeighbours.end());
	}

	size_t PosToIndex(b2Vec2 pos) const
	{
		size_t id = size_t(X * pos.x / this->width) + (size_t(Y*pos.y / this->height) * X);
		return id >= X*Y ? X*Y - 1u : id;
	}


	static AABB getAABB(T* e)
	{
		return getAABB(e->getPosition(), e->getShape()->getWidth(), e->getShape()->getHeight());
	}

	static AABB getAABB(T* e, b2Vec2 position)
	{
		return getAABB(position, e->getShape()->getWidth(), e->getShape()->getHeight());
	}
	static AABB getAABB(b2Vec2 position,float width, float height)
	{
		AABB aabb(position,position);
		b2Vec2 extent = {width * 0.5f, height * 0.5f};
		aabb.low -= extent;
		aabb.high += extent;
		return aabb;
	}

	void AddEntity(T* e)
	{
		if(!e) return;
		for(size_t index : AABBQuery(this,this->getAABB(e)))
		{
			this->cells[index].Entities.push_back(e);
		}
	}

	void UpdateEntity(T* e, b2Vec2 oldPos)
	{
		AABBQuery Old(this,getAABB(e, oldPos)), New(this,getAABB(e));
		auto s1 = Old.begin(), e1 = Old.end(), s2 = New.begin(), e2 = New.end();
		while(s1 != e1)
		{
			if(s2 == e2)
			{
				while(s1 != e1)
				{
					EraMove(this->cells[*s1].Entities, e);
					++s1;
				}
				return;
			}
			if(*s1 < *s2)
			{
				EraMove(this->cells[*s1].Entities, e);
				++s1;
			}
			else
			{
				if(*s2 < *s1)
				{
					this->cells[*s2].Entities.push_back(e);
				}
				else
				{
					++s1;
				}
				++s2;
			}
		}
		while(s2 != e2)
		{
			this->cells[*s2].Entities.push_back(e);
			++s2;
		}
	}

	const std::vector<T*>& getEntities(size_t index) const
	{
		return this->cells[index].Entities;
	}

	void RemoveEntity(T* e)
	{
		if(!e) return;
		for(size_t index : AABBQuery(this, this->getAABB(e)))
		{
			auto& ent = this->cells[index].Entities;
			ent.erase(std::remove(ent.begin(), ent.end(), e), ent.end());
		}
	}
};

