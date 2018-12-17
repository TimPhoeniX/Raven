#include "SteeringBehaviours.hpp"
#include "MovingObject.hpp"
#include <random>
#include <functional>
#include <vector>
#include "Utilities.hpp"
#include "World.hpp"
#include "Wall.hpp"
#include "Utils/Timing/sge_fps_limiter.hpp"

SteeringBehaviours::SteeringBehaviours(MovingObject* owner): owner(owner)
{
	this->neighbours.reserve(10u);
}


SteeringBehaviours::~SteeringBehaviours()
{}

b2Vec2 SteeringBehaviours::CalculateForce()
{
	constexpr float alone_time_max = 15.f;
	constexpr float wander_time_max = 15.f;
	float distCoef = b2Clamp(b2Distance(this->player->getPosition(), this->owner->getPosition()), 10.f, 100.f);
	distCoef = 1.f + (100.f - distCoef) * 0.05f;
	if(((this->total_space_time += SGE::delta_time) > 240.f))
		this->owner->setState(MoverState::Attacking);

	b2Vec2 sForce = b2Vec2_zero;
	sForce += 1.0f * this->WallAvoidance();
	sForce += 1.0f * this->ObstacleAvoidance();

	this->neighbours.clear();
	this->owner->getWorld()->getNeighbours(this->neighbours, this->owner, 5.f);
	if(this->neighbours.size() >= 5u)
	{
		this->owner->setState(MoverState::Attacking);
		for(auto element : this->neighbours)
		{
			element->setState(MoverState::Attacking);
		}
	}
	else
	{
		this->alone_time += SGE::delta_time;
	}

	if(this->owner->IsAttacking())
	{
		sForce += 1.f * this->Cohesion(this->neighbours);
		sForce += 1.5f * this->Alignment(this->neighbours);
		sForce += 2.f * this->Separation(this->neighbours);
		sForce += 2.f * this->Pursuit(this->player);
	}
	else
	{
		if(this->owner->IsMoving())
		{
			sForce += 2.5f * this->Wander();
			sForce += 2.5f * this->Hide(this->player, true, &obstacle);
			if(this->alone_time * distCoef > (alone_time_max + wander_time_max))
			{
				this->alone_time = 0.f;
				this->owner->setState(MoverState::Hiding);
				this->obstacle = nullptr;
			}
		}
		else
		{
			sForce += 2.f * this->Hide(this->player);
			if(this->alone_time * distCoef > (alone_time_max))
			{
				this->alone_time = 0.f;
				this->owner->setState(MoverState::Moving);
			}
		}
		sForce += 1.f * this->Separation(this->neighbours);
	}
	return 2.f * sForce;
}

b2Vec2 SteeringBehaviours::Seek(b2Vec2 target) const
{
	b2Vec2 desiredVel = target - owner->getPosition();
	desiredVel.Normalize();
	desiredVel *= owner->getMaxSpeed();
	return desiredVel - owner->getVelocity();
}

b2Vec2 SteeringBehaviours::Flee(b2Vec2 target) const
{
	if(b2DistanceSquared(target, owner->getPosition()) > 400.f)
	{
		return b2Vec2_zero;
	}
	b2Vec2 desiredVel = owner->getPosition() - target;
	desiredVel.Normalize();
	desiredVel *= owner->getMaxSpeed();
	return desiredVel - owner->getVelocity();
}

b2Vec2 SteeringBehaviours::Arrive(b2Vec2 target, Deceleration dec) const
{
	b2Vec2 toTarget = target - owner->getPosition();
	float dist = toTarget.Length();
	if(dist > 0.f)
	{
		constexpr float decTweak = .3f;
		float speed = dist / (float(dec)*decTweak);
		speed = b2Min(speed, owner->getMaxSpeed());
		toTarget *= speed / dist;
		return toTarget - owner->getVelocity();
	}
	return b2Vec2_zero;
}

b2Vec2 SteeringBehaviours::Pursuit(const MovingObject* const evader) const
{
	b2Vec2 toEvader = evader->getPosition() - owner->getPosition();
	float relHead = b2Dot(owner->getHeading(), evader->getHeading());
	if(b2Dot(toEvader, owner->getHeading()) > 0.f && relHead < -0.95f)
	{
		return this->Seek(evader->getPosition());
	}
	float lookAhead = toEvader.Length() / (owner->getMaxSpeed() + evader->getSpeed());
	return this->Seek(evader->getPosition() + lookAhead * evader->getVelocity());
}

b2Vec2 SteeringBehaviours::Evade(const MovingObject* const pursuer) const
{
	b2Vec2 toPursuer = pursuer->getPosition() - owner->getPosition();
	float lookAhead = toPursuer.Length() / (owner->getMaxSpeed() + pursuer->getSpeed());
	return Flee(pursuer->getPosition() + lookAhead * pursuer->getVelocity());
}

namespace
{
	auto randClamped = std::bind(std::uniform_real_distribution<float>(-1.f, 1.f),
								 std::default_random_engine());
}
b2Vec2 SteeringBehaviours::Wander()
{
	this->wTarget += b2Vec2{randClamped()*this->wJitter, randClamped()*this->wJitter};
	this->wTarget.Normalize();
	this->wTarget *= this->wRadius;
	b2Vec2 target = this->wTarget + b2Vec2{this->wDistance, 0.f};
	target = PointToWorldSpace(target, owner->getHeading(), owner->getPosition());
	return target - owner->getPosition();
}


b2Vec2 SteeringBehaviours::ObstacleAvoidance()
{
	this->boxLength = 4.f*(1.f + owner->getSpeed() / owner->getMaxSpeed());
	std::vector<SGE::Object*> obstacles = owner->getWorld()->getObstacles(owner, this->boxLength);
	SGE::Object* closestObject = nullptr;
	float closestDist = std::numeric_limits<float>::max();
	b2Vec2 closestLocalPos = b2Vec2_zero;
	for(SGE::Object* ob : obstacles)
	{
		b2Vec2 localPos = PointToLocalSpace(ob->getPosition(), owner->getHeading(), owner->getPosition());
		if(localPos.x >= 0)
		{
			float expRadius = owner->getShape()->getRadius() + ob->getShape()->getRadius();
			if(b2Abs(localPos.y) < expRadius)
			{
				float cX = localPos.x, cY = localPos.y;
				float sqrtPart = sqrt(expRadius*expRadius - cY*cY);
				float ip = cX - sqrtPart;
				if(ip <= 0.f) ip = cX + sqrtPart;
				if(ip < closestDist)
				{
					closestDist = ip;
					closestObject = ob;
					closestLocalPos = localPos;
				}
			}
		}
	}
	b2Vec2 sForce = b2Vec2_zero;
	if(closestObject)
	{
		float mp = 1.f + (this->boxLength - closestLocalPos.x) / this->boxLength;
		sForce.y = (closestObject->getShape()->getRadius() - closestLocalPos.y) * mp;
		constexpr float BrakingWeight = 0.2f;
		sForce.x = (closestObject->getShape()->getRadius() - closestLocalPos.x) * BrakingWeight;
	}
	return VectorToWorldSpace(sForce, owner->getHeading());
}

void SteeringBehaviours::CreateFeelers()
{
	this->feelers[0] = owner->getPosition() + 6.f * owner->getHeading();
	auto temp = b2Mul(b2Rot(b2_pi*0.25), owner->getHeading());
	this->feelers[1] = owner->getPosition() + 3.f * temp;
	temp = b2Mul(b2Rot(b2_pi*-0.25), owner->getHeading());
	this->feelers[2] = owner->getPosition() + 3.f * temp;
}

b2Vec2 SteeringBehaviours::WallAvoidance()
{
	this->CreateFeelers();
	float distToIp = 0.f;
	float closestDistToIp = std::numeric_limits<float>::max();
	Wall closestWall;
	b2Vec2 sForce = b2Vec2_zero;
	b2Vec2 point = b2Vec2_zero;
	b2Vec2 closestPoint = b2Vec2_zero;
	std::vector<std::pair<SGE::Object*, Wall>>& walls = owner->getWorld()->getWalls();
	for(b2Vec2 feeler : this->feelers)
	{
		for(auto& wall : walls)
		{
			if(LineIntersection(owner->getPosition(), feeler,
								wall.second.From(), wall.second.To(),
								distToIp, point))
			{
				if(distToIp < closestDistToIp)
				{
					closestDistToIp = distToIp;
					closestWall = wall.second;
					closestPoint = point;
				}
			}
		}
		if(closestPoint != b2Vec2_zero)
		{
			b2Vec2 overshoot = feeler - closestPoint;
			sForce = overshoot.Length() * closestWall.Normal();
		}
	}
	return sForce;
}

b2Vec2 SteeringBehaviours::Interpose(const MovingObject* const aA, const MovingObject* const aB) const
{
	b2Vec2 midPoint = 0.5*(aA->getPosition() + aB->getPosition());
	float eta = b2Distance(owner->getPosition(), midPoint) / owner->getMaxSpeed();
	b2Vec2 aPos = aA->getPosition() + eta * aA->getVelocity();
	b2Vec2 bPos = aB->getPosition() + eta * aB->getVelocity();
	midPoint = 0.5f*(aPos + bPos);
	return this->Arrive(midPoint, Deceleration::fast);
}

b2Vec2 SteeringBehaviours::GetHidingSpot(const b2Vec2& obPos, float obRadius, b2Vec2 targetPos)
{
	constexpr float boundaryDist = 2.f;
	float distAway = obRadius + boundaryDist;
	b2Vec2 toOb = obPos - targetPos;
	toOb.Normalize();
	return distAway * toOb + obPos;
}

b2Vec2 SteeringBehaviours::Hide(const MovingObject* const target, bool runaway, const SGE::Object** object) const
{
	float closestDist = !runaway ? std::numeric_limits<float>::max() : 0.f;
	b2Vec2 bestSpot = b2Vec2_zero;
	if(object && *object)
	{
		bestSpot = this->GetHidingSpot((*object)->getPosition(), (*object)->getShape()->getRadius(), target->getPosition());
		closestDist = b2DistanceSquared(bestSpot, owner->getPosition());
	}
	else
	{
		std::vector<SGE::Object*> obstacles = owner->getWorld()->getObstacles(owner, 30.f);
		for(SGE::Object* ob : obstacles)
		{
			b2Vec2 spot = this->GetHidingSpot(ob->getPosition(), ob->getShape()->getRadius(), target->getPosition());
			float dist = b2DistanceSquared(spot, owner->getPosition());
			if(!runaway ? dist < closestDist : dist > closestDist)
			{
				closestDist = dist;
				bestSpot = spot;
				if(object)
					*object = ob;
			}
		}
	}
	if(closestDist == std::numeric_limits<float>::max() || closestDist == 0.f)
	{
		return this->Evade(target);
	}
	return Arrive(bestSpot, Deceleration::fast);
}

b2Vec2 SteeringBehaviours::FollowPath()
{
	if(b2DistanceSquared(this->path.CurrentWaypoint(), owner->getPosition()) < this->WSDsq)
	{
		this->path.SetNextWaypoint();
	}
	if(!this->path.Finished())
	{
		return this->Seek(this->path.CurrentWaypoint());
	}
	else
	{
		return Arrive(this->path.CurrentWaypoint(), Deceleration::normal);
	}
}

b2Vec2 SteeringBehaviours::OffsetPursuit(const MovingObject* const leader, b2Vec2 offset) const
{
	b2Vec2 worldOffset = PointToWorldSpace(offset, leader->getHeading(), leader->getPosition());
	b2Vec2 toOffset = worldOffset - owner->getPosition();
	float lookAhead = toOffset.Length() / (owner->getMaxSpeed() + leader->getSpeed());
	return Arrive(worldOffset + lookAhead * leader->getVelocity(), Deceleration::fast);
}

b2Vec2 SteeringBehaviours::Separation(const std::vector<MovingObject*>& neighbours) const
{
	b2Vec2 sForce = b2Vec2_zero;
	for(MovingObject* neighbour : neighbours)
	{
		b2Vec2 toAgent = owner->getPosition() - neighbour->getPosition();
		float len = toAgent.Normalize();
		sForce += (1.f / len)*toAgent;
	}
	return sForce;
}

b2Vec2 SteeringBehaviours::Alignment(const std::vector<MovingObject*>& neighbours) const
{
	b2Vec2 avgHeading = b2Vec2_zero;
	for(MovingObject* neighbour : neighbours)
	{
		avgHeading += neighbour->getHeading();
	}
	if(!neighbours.empty())
	{
		avgHeading *= 1.f / neighbours.size();
		avgHeading -= owner->getHeading();
	}
	return avgHeading;
}

b2Vec2 SteeringBehaviours::Cohesion(const std::vector<MovingObject*>& neighbours) const
{
	b2Vec2 massCenter = b2Vec2_zero, sForce = b2Vec2_zero;
	for(MovingObject* neighbour : neighbours)
	{
		massCenter += neighbour->getPosition();
	}
	if(!neighbours.empty())
	{
		massCenter *= 1.f / neighbours.size();
		sForce = this->Seek(massCenter);
	}
	return sForce;
}
