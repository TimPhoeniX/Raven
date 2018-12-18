#include "Logics.hpp"
#include "Actions.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <Action/Actions/sge_action_move.hpp>
#include <IO/KeyboardState/sge_keyboard_state.hpp>
#include <Utils/Timing/sge_fps_limiter.hpp>
#include <Game/Director/sge_director.hpp>

#include "RavenScene.hpp"
#include "Utilities.hpp"
#include "Renderer/SpriteBatch/sge_sprite_batch.hpp"
#include "IntroScene.hpp"

void DamagePlayer::performLogic()
{
	this->ticked = false;
	if(this->timer > (this->player->getHP() > 10.f ? 1.f : 0.03f))
	{
		std::cout << "You have " << player->getHP() << " Health Points left." << std::endl;
		this->timer = 0;
		
	}
	this->movers.clear();
	this->world->getNeighbours(this->movers, this->player->getPosition(), 3.f * this->player->getShape()->getRadius());
	for (MovingObject* mover : this->movers)
	{
		float dist = b2DistanceSquared(this->player->getPosition(), mover->getPosition());
		float radii = this->player->getShape()->getRadius() + mover->getShape()->getRadius();
		radii *= radii;
		if(dist < radii)
		{
			if(!this->ticked)
			{
				this->timer += SGE::delta_time;
				this->ticked = true;
			}
			this->player->Damage(SGE::delta_time * this->dps);
		}
	}
}

void MoveAwayFromObstacle::performLogic()
{
	for(SGE::WorldElement& w: *this->obstacles)
	{
		SGE::Shape obShape = *w.getShape();
		switch(obShape.getType())
		{
		case SGE::ShapeType::Circle:
		{
			this->movers.clear();
			this->world->getNeighbours(this->movers, w.getPosition(), obShape.getRadius() + 1.f);		
			if(this->movers.empty()) continue;
			for(MovingObject* mo : this->movers)
			{
				SGE::Shape moShape = *mo->getShape();
				b2Vec2 toMover = mo->getPosition() - w.getPosition();
				float dist = toMover.Length();
				float radius = moShape.getRadius() + obShape.getRadius();
				if(dist > 0.f && dist < radius)
				{
					toMover *= (radius - dist) / dist;
					mo->setPosition(mo->getPosition() + toMover);
				}
			}
			break;
		}
		case SGE::ShapeType::Rectangle:
		{
			break;
		}
		case SGE::ShapeType::None: break;
		default: ;
		}
	}
	auto ob = this->world->getObstacles(this->player, 12.f);
	for(SGE::Object* o : ob)
	{
		SGE::Shape obShape = *o->getShape();
		switch(obShape.getType())
		{
		case SGE::ShapeType::Circle:
		{
			SGE::Shape moShape = *this->player->getShape();
			b2Vec2 toMover = this->player->getPosition() - o->getPosition();
			float dist = toMover.Length();
			float radius = moShape.getRadius() + obShape.getRadius();
			if(dist < radius)
			{
				toMover *= (radius - dist) / dist;
				this->player->setPosition(this->player->getPosition() + toMover);
			}
			break;
		}
		case SGE::ShapeType::Rectangle:
		{
			break;
		}
		case SGE::ShapeType::None: break;
		default:;
		}
	}
}

void SeparateZombies::performLogic()
{
	float baseRadius = 0.f;
	b2Vec2 basePosition = b2Vec2_zero;
	float otherRadius = 0.f;
	b2Vec2 otherPosition = b2Vec2_zero;
	for (MovingObject& mo : *this->movers)
	{
		baseRadius = mo.getShape()->getRadius();
		basePosition = mo.getPosition();
		this->colliding.clear();
		this->world->getNeighbours(this->colliding, &mo, 2.f * baseRadius);
		if (this->colliding.empty()) continue;
		for (MovingObject* other : this->colliding)
		{
			otherPosition = other->getPosition();
			otherRadius = other->getShape()->getRadius();
			b2Vec2 toOther = otherPosition - basePosition;
			float dist = toOther.Length();
			float radius = baseRadius + otherRadius;
			if (dist < radius)
			{
				toOther *= 0.5f * (radius - dist) / dist;
				other->setPosition(otherPosition + toOther);
				mo.setPosition(basePosition - toOther);
			}
		}
	}
	baseRadius = this->player->getShape()->getRadius();
	basePosition = this->player->getPosition();
	this->colliding.clear();
	this->world->getNeighbours(this->colliding, basePosition, 3.f * baseRadius);
	if (this->colliding.empty()) return;
	for (MovingObject* other : this->colliding)
	{
		otherPosition = other->getPosition();
		otherRadius = other->getShape()->getRadius();
		b2Vec2 toOther = otherPosition - basePosition;
		float dist = toOther.Length();
		float radius = baseRadius + otherRadius;
		if (dist < radius)
		{
			toOther *= 0.5f * (radius - dist) / dist;
			other->setPosition(otherPosition + toOther);
			this->player->setPosition(basePosition - toOther);
		}
	}
}

void MoveAwayFromWall::CollideWithWall(MovingObject& mo) const
{
	for (std::pair<SGE::Object*, Wall>& wall: this->world->getWalls())
	{
		float fradius = mo.getShape()->getRadius();
		b2Vec2 pos = mo.getPosition();
		if(PointToLineDistance(pos, wall.second.From(), wall.second.To()) < fradius)
		{
			float dist;
			b2Vec2 intersect;
			b2Vec2 radius;
			/*switch (wall.second.Type())
			{
			case Wall::Left: 
				radius = b2Vec2{+fradius,0.f};
				break;
			case Wall::Right:
				radius = b2Vec2{-fradius,0.f};
				break;
			case Wall::Top:
				radius = b2Vec2{0.f, -fradius};
				break;
			case Wall::Bottom:
				radius = b2Vec2{0.f, +fradius};
				break;
			default:
				continue;
			}*/
			radius = fradius * -wall.second.Normal();
			if(LineIntersection(pos, pos + radius, wall.second.From(), wall.second.To(), dist, intersect))
			{
				intersect -= pos + radius;
				mo.setPosition(pos + intersect);
			}
		}
	}
}

void MoveAwayFromWall::performLogic()
{
	for(MovingObject& mo: this->movers)
	{
		CollideWithWall(mo);
	}
	CollideWithWall(*this->player);
}

SnapCamera::SnapCamera(const float speed, const SGE::Key up, const SGE::Key down, const SGE::Key left, const SGE::Key right, const SGE::Key snapKey, SGE::Object* snapTo, SGE::Camera2d* cam)
	:Logic(SGE::LogicPriority::Low), speed(speed), up(up), down(down), left(left), right(right), snapKey(snapKey), snapTo(snapTo), cam(cam)
{}

void SnapCamera::performLogic()
{
	this->snapped = SGE::isPressed(snapKey); //We need to be able to send signals to actions, like sending actions to objects
	glm::vec2 move = {0, 0};
	if(!this->snapped)
	{
		move = this->snapTo->getPositionGLM();
		this->cam->setPositionGLM(move.x, move.y); //Replace with action, i.e. GoTo
	}
	else
	{
		if(SGE::isPressed(this->up)) move.y += this->speed;
		if(SGE::isPressed(this->down)) move.y -= this->speed;
		if(SGE::isPressed(this->right)) move.x += this->speed;
		if(SGE::isPressed(this->left)) move.x -= this->speed;
		this->sendAction(new SGE::ACTION::Move(this->cam, move.x, move.y, true));
	}
}

SpectatorCamera::SpectatorCamera(const float speed, const SGE::Key up, const SGE::Key down, const SGE::Key left, const SGE::Key right, SGE::Camera2d* cam)
	:Logic(SGE::LogicPriority::Low), speed(speed), up(up), down(down), left(left), right(right), cam(cam)
{
}

void SpectatorCamera::performLogic()
{
	glm::vec2 move = {0, 0};
	{
		if(SGE::isPressed(this->up)) move.y += this->speed;
		if(SGE::isPressed(this->down)) move.y -= this->speed;
		if(SGE::isPressed(this->right)) move.x += this->speed;
		if(SGE::isPressed(this->left)) move.x -= this->speed;
		this->sendAction(new SGE::ACTION::Move(this->cam, move.x, move.y, true));
	}
}

Timer::Timer(float time, SGE::Action* action): Logic(SGE::LogicPriority::Low), time(time), action(action)
{}

void Timer::performLogic()
{
	if(this->time > .0f)
	{
		this->time -= SGE::delta_time;
	}
	else
	{
		this->isOn = false;
		this->sendAction(this->action);
	}
}

OnKey::OnKey(SGE::Key key, SGE::Scene* scene): Logic(SGE::LogicPriority::Low), key(key), scene(scene)
{}

void OnKey::performLogic()
{
	if(SGE::isPressed(this->key))
	{
		this->sendAction(new Load(scene));
	}
}

Aim::Aim(World* world, SGE::Object* aimer, SGE::MouseObject* mouse, SGE::Camera2d* cam, std::size_t& counter, SGE::Object* pointer)
	: Logic(SGE::LogicPriority::High), world(world), aimer(aimer), mouse(mouse), cam(cam), counter(counter), pointer(pointer)
{
}

bool Aim::aim(b2Vec2 pos, b2Vec2 direction)
{
	b2Vec2 hitPos = b2Vec2_zero;
	MovingObject* hitObject = this->world->Raycast(pos, direction, hitPos);
	this->pointer->setVisible(true);
	b2Vec2 beam = hitPos - pos;
	this->pointer->setPosition(pos + 0.5f * beam);
	this->pointer->setOrientation(beam.Orientation());
	this->pointer->setShape(SGE::Shape::Rectangle(beam.Length(), 0.05f, true));

	if(hitObject)
	{
		//HitLogic
		hitObject->setState(MoverState::Dead);
		hitObject->setLayer(0.2f);
		this->world->RemoveMover(hitObject);
		++this->counter;
	}
	return nullptr != hitObject;
}

void Aim::performLogic()
{
	auto dir = this->cam->screenToWorld(this->mouse->getMouseCoords()) - this->aimer->getPositionGLM();
	b2Vec2 direction{dir.x, dir.y};
	direction.Normalize();
	this->aimer->setOrientation(direction.Orientation());
	if(reload > 0.f)
	{
		reload -= SGE::delta_time;
		if(reload < 0.f)
			this->pointer->setVisible(false);
	}
	if(this->fired)
	{
		this->fired = false;
		b2Vec2 pos = this->aimer->getPosition();
		aim(pos, direction);
	}
}

void Aim::Shoot()
{
	if (!this->fired && this->reload < 0)
	{
		this->fired = true;
		this->reload = 1.f;
	}
}

WinCondition::WinCondition(size_t& zombies, size_t& killedZombies, SGE::Scene* endGame, Player* player)
	: Logic(SGE::LogicPriority::Low), zombies(zombies), killedZombies(killedZombies), endGame(endGame), player(player)
{}

void WinCondition::performLogic()
{
	if(zombies == killedZombies || this->player->getHP() < 0)
	{
		reinterpret_cast<EndScene*>(endGame)->won = (zombies == killedZombies);
		this->sendAction(new Load(endGame));
	}
}
