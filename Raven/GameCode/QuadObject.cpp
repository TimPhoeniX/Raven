#include "QuadObject.hpp"

QuadObstacle::QuadObstacle(b2Vec2 pos, float rotation, std::array<b2Vec2, 4> vertices): Object(pos, true), vertices(vertices)
{
	this->visible = true;
	this->drawable = true;
	this->orientation = rotation;
	float radius = 0.f;
	b2Vec2 far;
	for (auto& vec : vertices)
	{
		float curRadius = vec.Length();
		if (curRadius > radius)
		{
			radius = curRadius;
			far = vec;
		}
		vec = this->position + b2Mul(b2Rot(this->orientation), vec);
		if(vec.x < aabbCache.low.x) aabbCache.low.x = vec.x;
		if(vec.y < aabbCache.low.y) aabbCache.low.y = vec.y;
		if(vec.x > aabbCache.high.x) aabbCache.high.x = vec.x;
		if(vec.y > aabbCache.high.y) aabbCache.high.y = vec.y;
	}
	this->Object::setShape(SGE::Shape::Quad(radius, 2.f * std::abs(far.x), 2.f * std::abs(far.y), true));
	edges[0] = Edge(vertices[0], vertices[3]);
	edges[1] = Edge(vertices[1], vertices[0]);
	edges[2] = Edge(vertices[2], vertices[1]);
	edges[3] = Edge(vertices[3], vertices[2]);
}

QuadObstacle::QuadObstacle(float x, float y, float rotation, std::array<b2Vec2, 4> vertices): QuadObstacle(b2Vec2{x,y}, rotation, vertices)
{
}

AABB QuadObstacle::getAABB() const
{
	return this->aabbCache;
}

const std::array<Edge, 4>& QuadObstacle::getEdges() const
{
	return this->edges;
}
