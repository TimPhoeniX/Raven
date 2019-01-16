#pragma once
#include <Object/sge_object.hpp>
#include "SteeringBehaviours.hpp"
#include <set>

class World;

enum class BotState: char
{
	Wandering,
	Attacking,
	Running,
	GettingAmmo,
	GettingHealth,
	GettingArmor
};

class RavenBot: public SGE::Object
{
protected:
	constexpr static float RailgunReload = 5.f;
	constexpr static float LauncherReload = 2.f;
	constexpr static float RailgunDamage = 100.f;
	constexpr static float LauncherDamage = 65.f;


	b2Vec2 velocity = b2Vec2_zero;
	b2Vec2 heading = b2Vec2_zero;
	b2Vec2 side = b2Vec2_zero;
	float mass = 1.f;
	float massInv = 1.f;
	float maxSpeed = 3.f;
	float maxForce = 15.f;
	float maxTurnRate = 90.f;
	float health = 100.f;
	float armor = 200.f;
	float rgCD = RailgunReload;
	float rlCD = LauncherReload;
	unsigned rgAmmo = 10u;
	unsigned rlAmmo = 15u;
	bool hit = false;
	World* world = nullptr;
	SteeringBehaviours* steering = new RavenSteering(this);
	BotState state = BotState::Wandering;
public:
	std::set<RavenBot*> enemies;
	SGE::Object* RailgunTrace = nullptr;

	RavenBot(const b2Vec2& position, SGE::Shape* shape, World* world, const b2Vec2& heading = b2Vec2{1.f,0.f})
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

	bool IsAttacking() const
	{
		return this->state == BotState::Attacking;
	}

	bool IsRunning() const
	{
		return this->state == BotState::Running;
	}

	bool IsWandering() const
	{
		return this->state == BotState::Wandering;
	}

	bool IsDead() const
	{
		return false;
	}

	void setState(BotState state)
	{
		this->state = state;
	}
	
	float Health() const
	{
		return this->health;
	}

	float Armor() const
	{
		return this->armor;
	}

	void AddHealth(float h)
	{
		this->health += h;
	}

	void AddArmor(float r)
	{
		if(armor < 0.f)
			armor = 0.f;
		this->armor += r;
	}

	void Damage(float d)
	{
		this->hit = true;
		if(armor < 0.f)
		{
			this->health -= d;
		}
		else
		{
			this->armor -= d;
		}
	}

	bool Hit() const
	{
		return this->hit;
	}

	void ClearHit()
	{
		this->hit = false;
	}

	unsigned RGAmmo() const
	{
		return this->rgAmmo;
	}

	unsigned RLAmmo() const
	{
		return this->rlAmmo;
	}

	float RLCD() const
	{
		return this->rlCD;
	}

	float RGCD() const
	{
		return this->rgCD;
	}

	void AddRailgunAmmo(unsigned i)
	{
		this->rgAmmo += i;
	}

	void AddRocketAmmo(unsigned i)
	{
		this->rlAmmo += i;
	}

	BotState getState()
	{
		return this->state;
	}

	bool IsFollowingPath() const
	{
		return !this->getSteering()->getPath().Finished();
	}
};
