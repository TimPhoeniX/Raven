#pragma once
#include "Box2D/Common/b2Math.h"

class Edge
{
protected:
	b2Vec2 from = b2Vec2_zero;
	b2Vec2 to = b2Vec2_zero;
public:
	Edge() = default;
	constexpr Edge(const Edge&) = default;
	Edge& operator=(const Edge&) = default;
	constexpr Edge(b2Vec2 from, b2Vec2 to) : from(from), to(to)
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

};

class Wall : public Edge
{
public:
	enum WallEdge
	{
		Left, Right, Top, Bottom, Invalid
	};
protected:
	WallEdge type = WallEdge::Invalid;
public:
	Wall() = default;
	constexpr Wall(const Wall&) = default;
	Wall& operator=(const Wall&) = default;
	constexpr Wall(b2Vec2 from, b2Vec2 to, WallEdge type): Edge(from, to), type(type)
	{}

	constexpr WallEdge Type() const
	{
		return this->type;
	}
};
