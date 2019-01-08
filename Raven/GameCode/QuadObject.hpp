#pragma once
#include <Object/sge_object.hpp>
#include <array>
#include "Wall.hpp"
#include "Utilities.hpp"

class QuadObstacle : public SGE::Object
{
private:
	static constexpr float InvRatio = 1.f / 64.f;
protected:
	std::array<b2Vec2, 4> vertices;
	std::array<Edge, 4> edges;
	AABB aabbCache;
public:
	QuadObstacle(b2Vec2 pos, float rotation, std::array<b2Vec2, 4> vertices);

	QuadObstacle(float x, float y, float rotation, std::array<b2Vec2, 4> vertices);

	QuadObstacle(b2Vec2 pos, float rotation, std::array<glm::vec2, 4> vertices) : QuadObstacle(pos, rotation,
		{ b2Vec2{ InvRatio * vertices[0].x, InvRatio * vertices[0].y}, { InvRatio * vertices[1].x, InvRatio * vertices[1].y }, { InvRatio * vertices[2].x, InvRatio * vertices[2].y }, { InvRatio * vertices[3].x, InvRatio * vertices[3].y } })
	{}
	QuadObstacle(float x, float y, float rotation, std::array<glm::vec2, 4> vertices) : QuadObstacle(b2Vec2{ x,y }, rotation,
		{ b2Vec2{ InvRatio * vertices[0].x, InvRatio * vertices[0].y },{ InvRatio * vertices[1].x, InvRatio * vertices[1].y }, { InvRatio * vertices[2].x, InvRatio * vertices[2].y }, { InvRatio * vertices[3].x, InvRatio * vertices[3].y } })
	{}

	AABB getAABB() const;
};
