#ifndef ZOMBIE_SCENE
#define ZOMBIE_SCENE

#include <Game/sge_game.hpp>
#include <Scene/sge_scene.hpp>
#include "MovingObject.hpp"
#include "World.hpp"

class QuadObstacle : public SGE::Object
{
private:
	static constexpr float InvRatio = 1.f / 64.f;
protected:
	std::array<Edge, 4> edges;
public:
	QuadObstacle(b2Vec2 pos, float rotation, std::array<b2Vec2, 4> vertices) : Object(pos, true)
	{
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
			vec = b2Mul(b2Rot(this->orientation), vec);
		}
		this->Object::setShape(SGE::Shape::Quad(radius, 2.f * std::abs(far.x), 2.f * std::abs(far.y), true));
		edges[0] = Edge(vertices[0], vertices[3]);
		edges[1] = Edge(vertices[1], vertices[0]);
		edges[2] = Edge(vertices[2], vertices[1]);
		edges[3] = Edge(vertices[3], vertices[2]);
	}
	QuadObstacle(float x, float y, float rotation, std::array<b2Vec2, 4> vertices) : QuadObstacle(b2Vec2{ x,y }, rotation, vertices)
	{};
	QuadObstacle(b2Vec2 pos, float rotation, std::array<glm::vec2, 4> vertices) : QuadObstacle(pos, rotation,
		{ b2Vec2{ InvRatio * vertices[0].x, InvRatio * vertices[0].y}, { InvRatio * vertices[1].x, InvRatio * vertices[1].y }, { InvRatio * vertices[2].x, InvRatio * vertices[2].y }, { InvRatio * vertices[3].x, InvRatio * vertices[3].y } })
	{};
	QuadObstacle(float x, float y, float rotation, std::array<glm::vec2, 4> vertices) : QuadObstacle(b2Vec2{ x,y }, rotation,
		{ b2Vec2{ InvRatio * vertices[0].x, InvRatio * vertices[0].y },{ InvRatio * vertices[1].x, InvRatio * vertices[1].y }, { InvRatio * vertices[2].x, InvRatio * vertices[2].y }, { InvRatio * vertices[3].x, InvRatio * vertices[3].y } })
	{};
};

namespace SGE
{
	class RealSpriteBatch;
}

class RavenScene : public SGE::Scene
{
protected:
	World world;
	SGE::Game* game = nullptr;
	std::string path;
	static bool init();
public:
	SGE::Scene* endScene = nullptr;

	RavenScene(SGE::Game* game, const char* path);

	virtual void loadScene() override;
	virtual void unloadScene() override;

	virtual ~RavenScene();

	virtual void finalize() override;

	virtual void onDraw() override;
};

#endif
