#ifndef ZOMBIEGAME_LOGICS
#define ZOMBIEGAME_LOGICS

#include <Logic/sge_logic.hpp>
#include <IO/Key/sge_key.hpp>
#include <Object/Camera2d/sge_camera2d.hpp>
#include <IO/Mouse/sge_mouse.hpp>
#include <vector>
#include "MovingObject.hpp"

namespace SGE
{
	class WorldElement;
}
class MovingObject;
class Player;
class World;

namespace SGE
{
	class Scene;
}

class DamagePlayer : public SGE::Logic
{
protected:
	World* world;
	Player* player;
	float dps;
	std::vector<MovingObject*> movers;
	float timer = 0.f;
	bool ticked = false;
public:
	explicit DamagePlayer(World* world, Player* player, float dps) : Logic(SGE::LogicPriority::High), world(world), player(player), dps(dps)
	{
		this->movers.reserve(15u);
	}

	void performLogic() override;
};

class MoveAwayFromObstacle : public SGE::Logic
{
protected:
	World* world;
	MovingObject* player;
	std::vector<SGE::WorldElement>* obstacles;
	std::vector<MovingObject*> movers;
public:

	MoveAwayFromObstacle(World* const world, MovingObject* player, std::vector<SGE::WorldElement>* const worldElements)
		: Logic(SGE::LogicPriority::Highest), world(world), player(player), obstacles(worldElements)
	{
		movers.reserve(10);
	}

	void performLogic() override;
};

class SeparateZombies : public SGE::Logic
{
protected:
	World* world;
	MovingObject* player;
	std::vector<MovingObject>* movers;
	std::vector<MovingObject*> colliding;
public:

	SeparateZombies(World* const world, MovingObject* player, std::vector<MovingObject>* const movers)
		: Logic(SGE::LogicPriority::Highest), world(world), player(player), movers(movers)
	{
		colliding.reserve(10);
	}

	void performLogic() override;
};

class MoveAwayFromWall : public SGE::Logic
{
protected:
	World* world;
	MovingObject* player;
	std::vector<MovingObject>& movers;
public:

	MoveAwayFromWall(World* const world, MovingObject* player, std::vector<MovingObject>& movers)
		: Logic(SGE::LogicPriority::Highest), world(world), player(player), movers(movers)
	{}

	void CollideWithWall(MovingObject& mo) const;
	void performLogic() override;
};

class SnapCamera : public SGE::Logic
{
	const float speed = 0;
	const SGE::Key up, down, left, right, snapKey;
	bool snapped = true;
	SGE::Object* snapTo = nullptr;
	SGE::Camera2d* cam = nullptr;

public:
	SnapCamera(const float specamed, const SGE::Key up, const SGE::Key down, const SGE::Key left, const SGE::Key right, const SGE::Key snapKey, SGE::Object* snapTo, SGE::Camera2d* cam);

	~SnapCamera() = default;

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
#endif
