#pragma once
#include "Box2D/Common/b2Math.h"
#include <array>
#include <vector>
#include "Path.hpp"

namespace SGE
{
	class Object;
}
class MovingObject;

enum class Deceleration:char
{
	slow = 3,
	normal = 2,
	fast = 1
};

class SteeringBehaviours
{
protected:
	std::array<b2Vec2, 3> feelers = {b2Vec2_zero, b2Vec2_zero, b2Vec2_zero};
	MovingObject* owner = nullptr;
	const MovingObject* player = nullptr;
	const SGE::Object* obstacle = nullptr;
	std::vector<MovingObject*> neighbours;
	b2Vec2 wTarget = b2Vec2_zero;
	Path path;
	float wRadius = 2.5f;
	float wDistance = 4.f;
	float wJitter = 15.f;
	float boxLength = 8.f;
	float32 WSDsq = 0.001f;
	float total_space_time = 0.0f;
	float alone_time = 0.0f;
	void CreateFeelers();
	static b2Vec2 GetHidingSpot(const b2Vec2& obPos, float obRadius, b2Vec2 targetPos);

public:
	SteeringBehaviours(MovingObject* owner);
	~SteeringBehaviours();
	b2Vec2 CalculateForce();
	b2Vec2 Seek(b2Vec2 target) const;
	b2Vec2 Flee(b2Vec2 target) const;
	b2Vec2 Arrive(b2Vec2 target, Deceleration dec) const;
	b2Vec2 Pursuit(const MovingObject* evader) const;
	b2Vec2 Evade(const MovingObject* pursuer) const;
	b2Vec2 Wander();
	b2Vec2 ObstacleAvoidance();
	b2Vec2 WallAvoidance();
	b2Vec2 Interpose(const MovingObject*const aA, const MovingObject*const aB) const;
	b2Vec2 Hide(const MovingObject*const target, bool runaway = false,const SGE::Object** = nullptr) const;
	b2Vec2 FollowPath();
	b2Vec2 OffsetPursuit(const MovingObject*const leader, b2Vec2 offset) const;
	b2Vec2 Separation(const std::vector<MovingObject*>& neighbours) const;
	b2Vec2 Alignment(const std::vector<MovingObject*>& neighbours) const;
	b2Vec2 Cohesion(const std::vector<MovingObject*>& neighbours) const;

	void setPlayer(const MovingObject*const player)
	{
		this->player = player;
	}

	std::vector<MovingObject*>& getNeighbours()
	{
		return this->neighbours;
	}
};

