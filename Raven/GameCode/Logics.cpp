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

MoveAwayFromObstacle::MoveAwayFromObstacle(World* const world, const std::vector<SGE::Object*>& obstacles): Logic(SGE::LogicPriority::Highest), world(world), obstacles(obstacles)
{
	movers.reserve(10);
}

void MoveAwayFromObstacle::performLogic()
{
	for(SGE::Object* w: this->obstacles)
	{
		SGE::Shape obShape = *w->getShape();
		this->world->getNeighbours(this->movers, w->getPosition(), obShape.getRadius() + 1.f);
		switch(obShape.getType())
		{
		case SGE::ShapeType::Rectangle:
		case SGE::ShapeType::Circle:
		{
			this->movers.clear();
			if(this->movers.empty()) continue;
			for(RavenBot* mo : this->movers)
			{
				SGE::Shape moShape = *mo->getShape();
				b2Vec2 toMover = mo->getPosition() - w->getPosition();
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
		case SGE::ShapeType::None: break;
		case SGE::ShapeType::Quad:
		{
			for(auto& wall : reinterpret_cast<QuadObstacle*>(w)->getEdges())
			{
				for(RavenBot* mo : this->movers)
				{
					float fradius = mo->getShape()->getRadius();
					b2Vec2 pos = mo->getPosition();
					if(PointToLineDistance(pos, wall.From(), wall.To()) < fradius)
					{
						float dist;
						b2Vec2 intersect;
						b2Vec2 radius = fradius * -wall.Normal();
						if(LineIntersection(pos, pos + radius, wall.From(), wall.To(), dist, intersect))
						{
							intersect -= pos + radius;
							mo->setPosition(pos + intersect);
						}
					}
				}
			}
		}
		default: break;
		}
	}
}

SeparateBots::SeparateBots(World* const world, std::vector<RavenBot>* const movers): Logic(SGE::LogicPriority::Highest), world(world), movers(movers)
{
	colliding.reserve(10);
}

void SeparateBots::performLogic()
{
	float baseRadius = 0.f;
	b2Vec2 basePosition = b2Vec2_zero;
	float otherRadius = 0.f;
	b2Vec2 otherPosition = b2Vec2_zero;
	for (RavenBot& mo : *this->movers)
	{
		baseRadius = mo.getShape()->getRadius();
		basePosition = mo.getPosition();
		this->colliding.clear();
		this->world->getNeighbours(this->colliding, &mo, 2.f * baseRadius);
		if (this->colliding.empty()) continue;
		for (RavenBot* other : this->colliding)
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
}

void MoveAwayFromWall::CollideWithWall(RavenBot& mo) const
{
	for (std::pair<SGE::Object*, Edge>& wall: this->world->getWalls())
	{
		float fradius = mo.getShape()->getRadius();
		b2Vec2 pos = mo.getPosition();
		if(PointToLineDistance(pos, wall.second.From(), wall.second.To()) < fradius)
		{
			float dist;
			b2Vec2 intersect;
			b2Vec2 radius = fradius * -wall.second.Normal();
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
	for(RavenBot& mo: this->movers)
	{
		CollideWithWall(mo);
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
	RavenBot* hitObject = this->world->Raycast(pos, direction, hitPos);
	this->pointer->setVisible(true);
	b2Vec2 beam = hitPos - pos;
	this->pointer->setPosition(pos + 0.5f * beam);
	this->pointer->setOrientation(beam.Orientation());
	this->pointer->setShape(SGE::Shape::Rectangle(beam.Length(), 0.05f, true));

	if(hitObject)
	{
		//HitLogic
		//hitObject->setState(BotState::Dead);
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
	if(false)
	{
		reinterpret_cast<EndScene*>(endGame)->won = (zombies == killedZombies);
		this->sendAction(new Load(endGame));
	}
}

RocketLogic::RocketLogic(std::vector<Rocket*> r, World* w): Logic(SGE::LogicPriority::High), rockets(r), world(w)
{
}

void RocketLogic::performLogic()
{
	for (Rocket* rocket: this->rockets)
	{
		b2Vec2 velocity = rocket->Speed() * rocket->Heading();
		auto oldPos = rocket->getPosition();
		rocket->setPosition(oldPos + SGE::delta_time * velocity);
		world->UpdateObstacle(rocket, oldPos);
	}
}

void BotLogic::updateBotState(RavenBot& bot)
{
	for(auto& enemy : this->gs->bots)
	{
		b2Vec2 botPos = bot.getPosition();
		b2Vec2 enemyPos = enemy.getPosition();
		b2Vec2 hit = enemyPos - botPos;
		if (&bot == &enemy) continue;
		if(bot.enemies.find(&enemy) != bot.enemies.end())
		{
			RavenBot* hitBot = this->world->Raycast(botPos, hit, hit);
			if (!hitBot)
			{
				bot.enemies.erase(hitBot);
			}
		}
		else
		{
			if (b2Abs(b2Atan2(b2Cross(bot.getHeading(), hit), b2Dot(bot.getHeading(), hit))) < 0.5f * b2_pi)
			{
				RavenBot* hitBot = this->world->Raycast(botPos, hit, hit);
				if (hitBot)
				{
					bot.enemies.insert(hitBot);
				}
			}
		}
	}

	switch (bot.getState())
	{
	case BotState::Wandering:
	{
		if(bot.enemies.empty())
		{
			if (bot.Health() < 65.f)
			{
				bot.setState(BotState::GettingHealth);
			}
			else if (bot.Armor() < 65.f)
			{
				bot.setState(BotState::GettingArmor);
			}
			else if( (bot.RGAmmo() + bot.RLAmmo()) < 15u)
			{
				bot.setState(BotState::GettingAmmo);
			}
		}
		else
		{
			if (bot.Health() < 65.f || bot.Armor() < 65.f)
			{
				bot.setState(BotState::Running);
			}
			else
			{
				bot.setState(BotState::Attacking);
			}
		}
	}
	case BotState::Attacking:
	{
		if (!bot.enemies.empty())
		{
			if (bot.Health() < 65.f || (bot.RGAmmo() + bot.RLAmmo()) == 0u)
			{
				bot.setState(BotState::Running);
			}
		}
		else
		{
			bot.setState(BotState::Wandering);
		}
	}
	case BotState::Running:
	{
		if (bot.enemies.empty())
		{
			bot.setState(BotState::Wandering);
		}
	}
	case BotState::GettingAmmo:
	{
		if(!bot.enemies.empty() || bot.Hit())
		{
			bot.setState(BotState::Attacking);
		}
		else
		{
			if((bot.RGAmmo() + bot.RLAmmo()) > 40u)
			{
				bot.setState(BotState::Wandering);
			}
		}
	}
	case BotState::GettingHealth:
	{
		if (!bot.enemies.empty())
		{
			bot.setState(BotState::Attacking);
		}
		else
		{
			if (bot.Hit())
			{
				bot.setState(BotState::Running);
			}
			else if (bot.Health() > 100.f)
			{
				bot.setState(BotState::Wandering);
			}
		}
	}
	case BotState::GettingArmor:
	{
		if (!bot.enemies.empty())
		{
			bot.setState(BotState::Attacking);
		}
		else
		{
			if(bot.Hit())
			{
				bot.setState(BotState::Running);
			}
			else if(bot.Armor() > 200.f)
			{
				bot.setState(BotState::Wandering);
			}
		}
	}
	default:
		break;
	}
	bot.ClearHit();
}

void BotLogic::updateBot(RavenBot& bot)
{
	this->updateBotState(bot);
	switch(bot.getState())
	{
	case BotState::Wandering:
		{
			if(!bot.IsFollowingPath())
			{
				GridVertex* begin = gs->GetCell(bot.getPosition())->vertex;
				GridVertex* end = gs->GetRandomVertex(bot.getPosition(), 5.f);
				bot.getSteering()->NewPath(std::move(this->gs->GetPath(begin, end)));
			}
		}
	case BotState::Attacking: break;
	case BotState::Running: break;
	case BotState::GettingAmmo: break;
	case BotState::GettingHealth: break;
	case BotState::GettingArmor: break;
	default: ;
	}
}

void BotLogic::performLogic()
{
	for(RavenBot& bot: this->gs->bots)
	{
		this->updateBot(bot);
	}
}
