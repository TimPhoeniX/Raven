#pragma once
#include <vector>
#include "Box2D/Common/b2Math.h"
#include "GridGraph.hpp"

class Path
{
	std::vector<b2Vec2> waypoints;
	b2Vec2 point;
public:
	Path(GridVertex* begin, GridVertex* end)
	{
		while(begin != end)
		{
			waypoints.push_back(end->Label().position);
			end = end->Parent();
		}
		this->point = this->waypoints.back();
	}
	Path() = default;
	Path(Path&&) = default;
	Path(const Path&) = default;
	Path& operator=(Path&&) = default;
	Path& operator=(const Path&) = default;
	~Path() = default;

	b2Vec2 CurrentWaypoint() const
	{
		return point;
	}
	void SetNextWaypoint()
	{
		this->waypoints.pop_back();
		if(!this->waypoints.empty())
			this->point = this->waypoints.back();
	}
	bool Finished() const
	{
		return waypoints.size() < 2u;
	}

	void Clear()
	{
		this->waypoints.clear();
	}

	bool Empty() const
	{
		return this->waypoints.empty();
	}

	b2Vec2 End() const
	{
		return !this->waypoints.empty() ? this->waypoints.front(): this->point;
	}
};
