#include "MovingObject.hpp"
#include "SteeringBehaviours.hpp"

void MovingObject::update(float delta)
{
	b2Vec2 sForce = this->getSteering()->CalculateForce();
	b2Vec2 acceleration = this->getMassInv() * sForce;
	b2Vec2 velocity = this->getVelocity() + delta * acceleration;
	velocity.Truncate(this->getMaxSpeed());
	this->setVelocity(velocity);
	this->setPosition(this->getPosition() + delta * getVelocity());
	if(this->getVelocity().LengthSquared() > 0.00000001f)
	{
		velocity.Normalize();
		this->setHeading(velocity);
		this->setSide(velocity.Skew());
	}
}
