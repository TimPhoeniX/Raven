#pragma once
#include <Object/sge_object.hpp>
#include "SteeringBehaviours.hpp"

class World;

enum class MoverState: char
{
	Moving,
	Hiding,
	Attacking,
	Dead
};

class MovingObject: public SGE::Object
{
protected:
	b2Vec2 velocity = b2Vec2_zero;
	b2Vec2 heading = b2Vec2_zero;
	b2Vec2 side = b2Vec2_zero;
	float mass = 1.f;
	float massInv = 1.f;
	float maxSpeed = 4.f;
	float maxForce = 16.f;
	float maxTurnRate = 90.f;
	World* world = nullptr;
	SteeringBehaviours* steering = new SteeringBehaviours(this);
	MoverState state = MoverState::Hiding;
public:

	MovingObject(const b2Vec2& position, SGE::Shape* shape, World* world, const b2Vec2& heading = b2Vec2{1.f,0.f})
		: Object(position, true, shape), heading(heading), side(heading.Skew()), world(world)
	{
		this->orientation = heading.Orientation();
	}

	b2Vec2 getVelocity() const
	{
		return velocity;
	}

	void setVelocity(b2Vec2 velocity)
	{
		this->velocity = velocity;
	}

	b2Vec2 getHeading() const
	{
		return heading;
	}

	void setHeading(b2Vec2 heading)
	{
		this->heading = heading;
		this->orientation = heading.Orientation();
	}

	b2Vec2 getSide() const
	{
		return side;
	}

	void setSide(b2Vec2 side)
	{
		this->side = side;
	}

	float getMass() const
	{
		return mass;
	}

	void setMass(float mass)
	{
		this->mass = mass;
		this->massInv = 1.f / mass;
	}

	float getMassInv() const
	{
		return massInv;
	}

	float getMaxSpeed() const
	{
		return maxSpeed;
	}

	void setMaxSpeed(float maxSpeed)
	{
		this->maxSpeed = maxSpeed;
	}

	float getMaxForce() const
	{
		return maxForce;
	}

	void setMaxForce(float maxForce)
	{
		this->maxForce = maxForce;
	}

	float getMaxTurnRate() const
	{
		return maxTurnRate;
	}

	void setMaxTurnRate(float maxTurnRate)
	{
		this->maxTurnRate = maxTurnRate;
	}

	World* getWorld() const
	{
		return world;
	}

	SteeringBehaviours* getSteering() const
	{
		return steering;
	}

	void setSteering(SteeringBehaviours* steering)
	{
		this->steering = steering;
	}

	void update(float delta);

	float getSpeed() const
	{
		return this->velocity.Length();
	}

	bool IsDead() const
	{
		return this->state == MoverState::Dead;
	}

	bool IsAttacking() const
	{
		return this->state == MoverState::Attacking;
	}

	bool IsHiding() const
	{
		return this->state == MoverState::Hiding;
	}

	bool IsMoving() const
	{
		return this->state == MoverState::Moving;
	}

	void setState(MoverState state)
	{
		this->state = state;
	}
};

class Player: public MovingObject
{
protected: 
	float hp = 100.f;
public:
	using MovingObject::MovingObject;
	float getHP() const
	{
		return this->hp;
	}
	void Damage(float damage)
	{
		this->hp -= damage;
	}
};