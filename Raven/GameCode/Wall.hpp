#pragma once
#include "Box2D/Common/b2Math.h"

class Wall
{
public:
	enum WallEdge
	{
		Left, Right, Top, Bottom
	};
private:
	b2Vec2 from;
	b2Vec2 to;
	WallEdge type;
public:
	Wall() = default;
	constexpr Wall(const Wall&) = default;
	constexpr Wall(b2Vec2 from, b2Vec2 to, WallEdge type): from(from), to(to), type(type)
	{}
	constexpr b2Vec2 From() const
	{
		return from;
	}
	constexpr b2Vec2 To() const
	{
		return to;
	}
	b2Vec2 Normal() const
	{
		auto norm = (to - from).Skew();
		norm.Normalize();
		return norm;
	}

	constexpr WallEdge Type() const
	{
		return this->type;
	}
};
