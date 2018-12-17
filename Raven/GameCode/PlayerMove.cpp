#include "PlayerMove.hpp"
#include <Utils/Timing/sge_fps_limiter.hpp>
#include <Box2D/Common/b2Math.h>
#include <IO/KeyboardState/sge_keyboard_state.hpp>
#include "MovingObject.hpp"

PlayerMove::PlayerMove(MovingObject* object, const float speed, const SGE::Key up, const SGE::Key down, const SGE::Key left, const SGE::Key right)
	: Logic(SGE::LogicPriority::Highest), speed(speed), up(up), down(down), left(left), right(right), object(object)
{}

void PlayerMove::performLogic()
{
	b2Vec2 move = b2Vec2_zero;
	if(isPressed(this->up)) move.y += 1;
	if(isPressed(this->down)) move.y -= 1;
	if(isPressed(this->right)) move.x += 1;
	if(isPressed(this->left)) move.x -= 1;
	if(b2Vec2_zero != move)
	{
		move.Normalize();
		if(SGE::isPressed(SGE::Key::LShift))
			this->object->setPosition(this->object->getPosition() + SGE::delta_time * 3.f * speed * move);
		else
			this->object->setPosition(this->object->getPosition() + SGE::delta_time * speed * move);
	}
}