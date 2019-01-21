#ifndef ZOMBIEGAME_LOGICS
#define ZOMBIEGAME_LOGICS

#include <Logic/sge_logic.hpp>
#include <IO/Key/sge_key.hpp>
#include <Object/Camera2d/sge_camera2d.hpp>
#include <IO/Mouse/sge_mouse.hpp>
#include <Utils/Timing/sge_fps_limiter.hpp>
#include <vector>
#include <random>

#include "RavenBot.hpp"
#include "Objects.hpp"
#include "World.hpp"

namespace SGE
{
	class WorldElement;
}
class RavenGameState;
class RavenBot;
class Player;
class World;

namespace SGE
{
	class Scene;
}

class MoveAwayFromObstacle : public SGE::Logic
{
protected:
	World* world;
	std::vector<SGE::Object*> obstacles;
	std::vector<RavenBot*> movers;
public:
	MoveAwayFromObstacle(World* const world, const std::vector<SGE::Object*>& obstacles);

	void performLogic() override;
};

class SeparateBots : public SGE::Logic
{
protected:
	World* world;
	std::vector<RavenBot>* movers;
	std::vector<RavenBot*> colliding;
public:

	SeparateBots(World* const world, std::vector<RavenBot>* const movers);

	void performLogic() override;
};

class MoveAwayFromWall : public SGE::Logic
{
protected:
	World* world;
	std::vector<RavenBot>& movers;
public:

	MoveAwayFromWall(World* const world, std::vector<RavenBot>& movers)
		: Logic(SGE::LogicPriority::Highest), world(world), movers(movers)
	{}

	void CollideWithWall(RavenBot& mo) const;
	void performLogic() override;
};

class SpectatorCamera: public SGE::Logic
{
	const float speed = 0;
	const SGE::Key up, down, left, right;
	SGE::Camera2d* cam = nullptr;

public:
	SpectatorCamera(const float specamed, const SGE::Key up, const SGE::Key down, const SGE::Key left, const SGE::Key right, SGE::Camera2d* cam);

	~SpectatorCamera() = default;

	void performLogic() override;
};

class Timer : public SGE::Logic
{
	float time = .0f;
	SGE::Action* action = nullptr;
public:
	Timer(float time, SGE::Action* action);
	void performLogic() override;
};

class OnKey : public SGE::Logic
{
	SGE::Key key;
	SGE::Scene* scene = nullptr;
public:
	OnKey(SGE::Key key, SGE::Scene* scene);
	void performLogic() override;
};

namespace SGE
{
	class Scene;
}

class RocketLogic: public SGE::Logic
{
protected:
	RavenGameState* gs;
	World* world;
public:
	RocketLogic(RavenGameState* gs, World* w);

	void performLogic() override;
};

class RavenGameState;

class BotLogic: public SGE::Logic
{
protected:
	World* world;
	RavenGameState* gs;

	void updateEnemies(RavenBot& bot);
	void updateItems(RavenBot& bot);
	void updateState(RavenBot& bot);
	void pickItems(RavenBot& bot);
	void ResetBot(RavenBot& bot);
	void updateBotState(RavenBot& bot);
	void FireRG(RavenBot& bot);
	void FireRL(RavenBot& bot);
	void UpdateEnemy(RavenBot& bot);
	void GetItem(RavenBot& bot, Item::IType type);
	void updateBot(RavenBot& bot);
	
	std::function<float(void)> randAngle;
public:
	BotLogic(World* world, RavenGameState* gs)
		: Logic(SGE::LogicPriority::Highest), world(world), gs(gs)
	{
		constexpr float spread = 0.01f;
		randAngle = std::bind(std::uniform_real_distribution<float>{-spread * b2_pi, spread * b2_pi}, std::default_random_engine{std::random_device{}()});
	}

	void performLogic() override;
};

class ItemLogic: public SGE::Logic
{
protected:
	World* world;
	RavenGameState* gs;
public:
	ItemLogic(World* world, RavenGameState* gs)
		: Logic(SGE::LogicPriority::High), world(world), gs(gs){}
	
	void performLogic() override;
};

#endif
