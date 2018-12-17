#include "SteeringBehavioursUpdate.hpp"
#include "Utils/Timing/sge_fps_limiter.hpp"
#include "Box2D/Common/b2Math.h"
#include "MovingObject.hpp"
#include "World.hpp"

SteeringBehavioursUpdate::SteeringBehavioursUpdate(std::vector<MovingObject>* objects): Logic(SGE::LogicPriority::Highest), objects(objects)
{}

void SteeringBehavioursUpdate::performLogic()
{
	for(MovingObject& o : *this->objects)
	{
		if(o.IsDead())
		{
			continue;
		}
		b2Vec2 sForce = o.getSteering()->CalculateForce();
		sForce.Truncate(o.getMaxForce());
		b2Vec2 acceleration = o.getMassInv() * sForce;
		b2Vec2 velocity = o.getVelocity() + SGE::delta_time * acceleration;
		velocity.Truncate(o.getMaxSpeed());
		o.setVelocity(velocity);
		auto oldPos = o.getPosition();
		o.setPosition(oldPos + SGE::delta_time * o.getVelocity());
		if(o.getVelocity().LengthSquared() > 0.00000001f)
		{
			velocity.Normalize();
			o.setHeading(velocity);
			o.setSide(velocity.Skew());
		}
		o.getWorld()->UpdateMover(&o, oldPos);
	}
}