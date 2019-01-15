#pragma once
#include <vector>
#include "Box2D/Common/b2Math.h"
#include "GridGraph.hpp"

class Path
{
	std::vector<b2Vec2> waypoints;
public:
	Path(GridVertex* begin, GridVertex* end)
	{
		while(begin != end)
		{
			waypoints.push_back(end->Label().position);
			end = end->Parent();
		}
	}
	Path() = default;
	Path(Path&&) = default;
	Path(const Path&) = default;
	Path& operator=(Path&&) = default;
	Path& operator=(const Path&) = default;
	~Path() = default;

	b2Vec2 CurrentWaypoint() const
	{
		return waypoints.back();
	}
	void SetNextWaypoint()
	{
		this->waypoints.pop_back();
	}
	bool Finished() const
	{
		return waypoints.size() <= 1u;
	}

	void Clear()
	{
		this->waypoints.clear();
	}
};
