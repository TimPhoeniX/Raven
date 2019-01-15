#ifndef ZOMBIEGAME_LOGICS
#define ZOMBIEGAME_LOGICS

#include <Logic/sge_logic.hpp>
#include <IO/Key/sge_key.hpp>
#include <Object/Camera2d/sge_camera2d.hpp>
#include <IO/Mouse/sge_mouse.hpp>
#include <Utils/Timing/sge_fps_limiter.hpp>
#include <vector>
#include "RavenBot.hpp"
#include "Objects.hpp"
#include "World.hpp"

namespace SGE
{
	class WorldElement;
}
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

class Aim : public SGE::Logic
{
protected:
	World* world;
	SGE::Object* aimer;
	SGE::MouseObject* mouse;
	SGE::Camera2d* cam;
	float reload = -1.f;
	std::size_t& counter;
	bool fired = false;
	SGE::Object* pointer;
	bool aim(b2Vec2 pos, b2Vec2 target);
public:
	Aim(World* world, SGE::Object* aimer, SGE::MouseObject* mouse, SGE::Camera2d* cam, std::size_t& counter, SGE::Object*);
	void performLogic() override;
	void Shoot();
};

namespace SGE
{
	class Scene;
}

class WinCondition : public SGE::Logic
{
protected:
	volatile size_t& zombies;
	volatile size_t& killedZombies;
	SGE::Scene* endGame = nullptr;
	Player* player;
public:
	WinCondition(size_t& zombies, size_t& killedZombies, SGE::Scene* endGame, Player* player);
	virtual void performLogic() override;
};

class RocketLogic: public SGE::Logic
{
protected:
	std::vector<Rocket*>& rockets;
	World* world;
public:
	RocketLogic(std::vector<Rocket*> r, World* w);

	void performLogic() override;
};

class RavenGameState;

class BotLogic: public SGE::Logic
{
protected:
	World* world;
	RavenGameState* gs;

public:
	explicit BotLogic(World* world, RavenGameState* gs)
		: Logic(SGE::LogicPriority::Highest), world(world), gs(gs)
	{
	}

	void updateBotState(RavenBot& bot);
	void updateBot(RavenBot& bot);
	void performLogic() override;
};
#endif
