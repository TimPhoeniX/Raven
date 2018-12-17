#pragma once
#include "Box2D/Common/b2Math.h"

class Path
{
public:
	b2Vec2 CurrentWaypoint() const
	{
		return b2Vec2_zero;
	}
	void SetNextWaypoint()
	{}
	bool Finished() const
	{
		return true;
	}
};
