#include "Logics.hpp"
#include "Actions.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <Action/Actions/sge_action_move.hpp>
#include <IO/KeyboardState/sge_keyboard_state.hpp>
#include <Utils/Timing/sge_fps_limiter.hpp>
#include <Game/Director/sge_director.hpp>
#include <Renderer/SpriteBatch/sge_sprite_batch.hpp>

#include "RavenScene.hpp"
#include "Utilities.hpp"
#include "IntroScene.hpp"
#include "Box2D/Dynamics/Contacts/b2ChainAndCircleContact.h"

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
						else if(b2DistanceSquared(wall.From(), pos) < fradius * fradius)
						{
							radius = pos - wall.From();
							float pen = radius.Normalize();
							mo->setPosition(pos + ((fradius - pen) * radius));
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

RocketLogic::RocketLogic(RavenGameState* gs, World* w): Logic(SGE::LogicPriority::High), gs(gs), world(w)
{
}

void RocketLogic::performLogic()
{
	if(this->gs->rockets.empty()) return;
	for (Rocket* rocket: this->gs->rockets)
	{
		b2Vec2 velocity = rocket->Speed() * rocket->Heading();
		auto oldPos = rocket->getPosition();
		rocket->setPosition(oldPos + SGE::delta_time * velocity);
		this->world->UpdateRocket(rocket, oldPos);
		std::vector<RavenBot*> bots;
		constexpr float hitRadius = 0.5f * Rocket::Height();
		b2Vec2 hitSpot = rocket->getPosition() + (hitRadius * rocket->Heading());
		this->world->getNeighbours(bots, hitSpot, hitRadius);
		if(!bots.empty())
		{
			rocket->Prime();
			continue;
		}
		auto obstacles = this->world->getObstacles(hitSpot, hitRadius);
		for(SGE::Object* ob : obstacles)
		{
			if(ob == rocket) continue;
			if(rocket->IsPrimed()) break;
			auto& obShape = *ob->getShape();
			switch(obShape.getType())
			{
			case SGE::ShapeType::Rectangle:
			{
				b2Vec2 hitVec = PointToLocalSpace(hitSpot, b2Mul(b2Rot(ob->getOrientation()), {1.f, 0.f}), ob->getPosition());
				b2Vec2 halves = 0.5f * ob->getScale();
				b2Vec2 penPoint = b2Clamp(hitVec, -halves, halves);
				if(b2DistanceSquared(penPoint, hitSpot) < hitRadius * hitRadius)
				{
					rocket->Prime();
				}
				break;
			}
			case SGE::ShapeType::Circle:
			{
				b2Vec2 toMover = hitSpot - ob->getPosition();
				float dist = toMover.Length();
				float radius = hitRadius + obShape.getRadius();
				if(dist < radius)
				{
					rocket->Prime();
				}
				break;
			}
			case SGE::ShapeType::None: break;
			case SGE::ShapeType::Quad:
			{
				for(auto& wall : reinterpret_cast<QuadObstacle*>(ob)->getEdges())
				{
					if(PointToLineDistance(hitSpot, wall.From(), wall.To()) < hitRadius)
					{
						float dist;
						b2Vec2 intersect;
						b2Vec2 radius = hitRadius * -wall.Normal();
						if(LineIntersection(hitSpot, hitSpot + radius, wall.From(), wall.To(), dist, intersect))
						{
							rocket->Prime();
						}
					}
				}
				break;
			}
			default: break;
			}
		}
		if(rocket->IsPrimed()) continue;
		for(std::pair<SGE::Object*, Edge>& wall : this->world->getWalls())
		{
			if(PointToLineDistance(hitSpot, wall.second.From(), wall.second.To()) < hitRadius)
			{
				float dist;
				b2Vec2 intersect;
				b2Vec2 radius = hitRadius * -wall.second.Normal();
				if(LineIntersection(hitSpot, hitSpot + radius, wall.second.From(), wall.second.To(), dist, intersect))
				{
					rocket->Prime();
					break;
				}
			}
		}
	}
	std::stack<Rocket*> primed;
	for(Rocket* rocket : this->gs->rockets)
	{
		if(!rocket->IsPrimed()) continue;
		primed.push(rocket);
	}
	while(!primed.empty())
	{
		Rocket* rocket = primed.top();
		std::vector<RavenBot*> bots;
		this->world->getNeighbours(bots, rocket->getPosition(), Rocket::Radius());
		for(RavenBot* bot : bots)
		{
			bot->Damage(RavenBot::LauncherDamage);
		}
		std::vector<Rocket*> rockets(this->world->getRockets(rocket->getPosition(), Rocket::Radius()));
		for(Rocket* otherRocket : rockets)
		{
			otherRocket->Prime();
		}
		this->gs->RemoveRocket(rocket);
		primed.pop();
	}
	for(Rocket* explosion: this->gs->explosions)
	{
		if(explosion->RemainingTime() > 0.f)
		{
			explosion->Expire(SGE::delta_time);
		}
		else
		{
			primed.push(explosion);
		}
	}
	while(!primed.empty())
	{
		Rocket* rocket = primed.top();
		this->gs->RemoveExplosion(rocket);
		primed.pop();
	}
}

void BotLogic::updateEnemies(RavenBot& bot)
{
	for(auto& enemy : this->gs->bots)
	{
		if(&bot == &enemy) continue;
		b2Vec2 botPos = bot.getPosition();
		b2Vec2 enemyPos = enemy.getPosition();
		b2Vec2 hit = enemyPos - botPos;
		hit.Normalize();
		if(bot.enemies.find(&enemy) != bot.enemies.end())
		{
			RavenBot* hitBot = this->world->RaycastBot(&bot, botPos, hit, hit);
			if(!hitBot)
			{
				bot.enemies.erase(&enemy);
				if(&enemy == bot.getSteering()->getEnemy())
					bot.getSteering()->setEnemy(nullptr);
			}
		}
		else
		{
			if (b2Abs(b2Atan2(b2Cross(bot.getHeading(), hit), b2Dot(bot.getHeading(), hit))) < 0.25f * b2_pi)
			{
				RavenBot* hitBot = this->world->RaycastBot(&bot, botPos, hit, hit);
				if(hitBot)
				{
					bot.enemies.insert(hitBot);
				}
			}
		}
	}
}

void BotLogic::updateItems(RavenBot& bot)
{
	for(Item* item : this->gs->items)
	{
		b2Vec2 botPos = bot.getPosition();
		b2Vec2 itemPos = item->getPosition();
		b2Vec2 hit = itemPos - botPos;
		if(bot.items.find(item) == bot.items.end())
		{
			if(b2Abs(b2Atan2(b2Cross(bot.getHeading(), hit), b2Dot(bot.getHeading(), hit))) < 0.25f * b2_pi)
			{
				Item* hitItem = this->world->RaycastItem(botPos, hit, hit);
				if(hitItem)
				{
					bot.items.insert(hitItem);
				}
			}
		}
	}
}

void BotLogic::updateState(RavenBot& bot)
{
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
				if (bot.Health() < 65.f || bot.Armor() < 65.f || (bot.RLAmmo() + bot.RGAmmo()) == 0u)
				{
					bot.setState(BotState::Running);
				}
				else
				{
					bot.setState(BotState::Attacking);
				}
			}
			break;
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
			break;
		}
	case BotState::Running:
		{
			if(bot.enemies.empty())
			{
				bot.setState(BotState::Wandering);
			}
			break;
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
			break;
		}
	case BotState::GettingHealth:
		{
			if (!bot.enemies.empty() || bot.Hit())
			{
				bot.setState(BotState::Attacking);
			}
			else
			{
				if (bot.Health() > 100.f)
				{
					bot.setState(BotState::Wandering);
				}
			}
			break;
		}
	case BotState::GettingArmor:
		{
			if (!bot.enemies.empty(), bot.Hit())
			{
				bot.setState(BotState::Attacking);
			}
			else
			{
				if(bot.Armor() > 200.f)
				{
					bot.setState(BotState::Wandering);
				}
			}
			break;
		}
	default:
		break;
	}
}

void BotLogic::pickItems(RavenBot& bot)
{
	auto items = this->world->getItems(&bot);
	for(Item* item : items)
	{
		item->useItem(bot);
		this->gs->UseItem(item);
	}
}

void BotLogic::ResetBot(RavenBot& bot)
{
	b2Vec2 newPos = this->gs->GetRandomVertex(bot.getPosition(), 30.f, false)->Label().position;
	bot.Respawn(newPos);
	for(auto& enemy : this->gs->bots)
	{
		enemy.enemies.erase(&bot);
		if(&bot == enemy.getSteering()->getEnemy())
		{
			enemy.getSteering()->setEnemy(nullptr);
		}
	}
}

void BotLogic::updateBotState(RavenBot& bot)
{
	if(bot.IsDead())
	{
		this->ResetBot(bot);
	}

	this->updateEnemies(bot);
	this->pickItems(bot);
	this->updateItems(bot);
	this->updateState(bot);
	
	bot.Reloading(SGE::delta_time);
	bot.ClearHit();
}

void BotLogic::FireRG(RavenBot& bot)
{
	if(!bot.FireRG()) return;
	b2Vec2 pos = bot.getPosition();
	b2Vec2 direction = bot.getHeading();
	direction = b2Mul(b2Rot(this->randAngle()), direction);
	b2Vec2 hitPos = b2Vec2_zero;
	RavenBot* hitObject = this->world->RaycastBot(&bot, pos, direction, hitPos);
	bot.RailgunTrace->setVisible(true);
	b2Vec2 beam = hitPos - pos;
	bot.RailgunTrace->setPosition(pos + 0.5f * beam);
	bot.RailgunTrace->setOrientation(beam.Orientation());
	bot.RailgunTrace->setShape(SGE::Shape::Rectangle(beam.Length(), 0.1f, true));

	if(hitObject)
	{
		hitObject->Damage(RavenBot::RailgunDamage);
	}
}

void BotLogic::FireRL(RavenBot& bot)
{
	if(!bot.FireRL()) return;
	b2Vec2 pos = bot.getPosition();
	b2Vec2 direction = bot.getHeading();
	direction = b2Mul(b2Rot(this->randAngle()), direction);
	this->gs->NewRocket(pos, direction);
}

void BotLogic::UpdateEnemy(RavenBot& bot)
{
	float dist = std::numeric_limits<float>::max();
	b2Vec2 pos = bot.getPosition();
	RavenBot* target = nullptr;
	for(auto enemy : bot.enemies)
	{
		float enemyDist = b2DistanceSquared(pos, enemy->getPosition());
		if( enemyDist < dist)
		{
			dist = enemyDist;
			target = enemy;
		}
	}
	if(target)
	{
		bot.getSteering()->setEnemy(target);
	}
}

void BotLogic::GetItem(RavenBot& bot, Item::IType type)
{
	b2Vec2 pos = bot.getPosition();
	Item* closestItem = nullptr;
	float distance = std::numeric_limits<float>::max();
	for(Item* item : bot.items)
	{
		if(item->Type() == type)
		{
			float newDist = b2DistanceSquared(pos, item->getPosition());
			if(newDist < distance)
			{
				closestItem = item;
				distance = newDist;
			}
		}
	}
	if(closestItem)
	{
		if(!bot.IsFollowingPath())
		{
			GridVertex* begin = gs->GetVertex(pos);
			GridVertex* end = gs->GetVertex(closestItem->getPosition());
			bot.getSteering()->NewPath(std::move(this->gs->GetPath(begin, end)));
		}
		else if(b2DistanceSquared(closestItem->getPosition(), bot.getSteering()->getPath().End()) > 0.1)
		{
			GridVertex* begin = gs->GetVertex(pos);
			GridVertex* end = gs->GetVertex(closestItem->getPosition());
			bot.getSteering()->NewPath(std::move(this->gs->GetPath(begin, end)));
		}
	}
	else
	{
		if(!bot.IsFollowingPath())
		{
			GridVertex* begin = gs->GetVertex(pos);
			GridVertex* end = gs->GetRandomVertex(pos,25,true);
			bot.getSteering()->NewPath(std::move(this->gs->GetPath(begin, end)));
		}
	}
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
			GridVertex* begin = gs->GetVertex(bot.getPosition());
			GridVertex* end = gs->GetRandomVertex(bot.getPosition(),25.f,true);
			bot.getSteering()->NewPath(std::move(this->gs->GetPath(begin, end)));
		}
		break;
	}
	case BotState::Attacking:
	{
		if(!bot.getSteering()->getEnemy())
		{
			UpdateEnemy(bot);
		}
		const RavenBot* enemy = bot.getSteering()->getEnemy();
		if(!enemy) break;
		b2Vec2 direction = enemy->getPosition() - bot.getPosition();
		float distance = direction.Normalize();
		bot.setHeading(direction);
		if(bot.CanFireRG())
		{
			this->FireRG(bot);
		}
		else if(bot.CanFireRL() && distance > Rocket::Radius())
		{
			this->FireRL(bot);
		};
		break;
	}
	case BotState::Running:
	{
		if(!bot.getSteering()->getEnemy())
		{
			UpdateEnemy(bot);
		}
		const RavenBot* enemy = bot.getSteering()->getEnemy();
		if(!enemy || !bot.IsFollowingPath()) break;
		GridVertex* begin = this->gs->GetVertex(bot.getPosition());
		GridVertex* end = this->gs->GetRandomVertex(enemy->getPosition(),30.f,false);
		bot.getSteering()->NewPath(this->gs->GetPath(begin, end));
		break;
	}
	case BotState::GettingAmmo:
	{
		if(bot.RGAmmo() < bot.RLAmmo())
		{
			this->GetItem(bot, Item::IType::RGAmmo);
		}
		else
		{
			this->GetItem(bot, Item::IType::RLAmmo);
		}
		break;
	}
	case BotState::GettingHealth:
	{
		this->GetItem(bot, Item::IType::Health);
		break;
	}
	case BotState::GettingArmor:
	{
		this->GetItem(bot, Item::IType::Armor);
		break;
	}
	default: break;
	}
}

void BotLogic::performLogic()
{
	for(RavenBot& bot: this->gs->bots)
	{
		this->updateBot(bot);
	}
}

void ItemLogic::performLogic()
{
	for(Item* item : this->gs->items)
	{
		if(item->Respawnable())
		{
			item->Respawn(this->gs->GetRandomVertex(item->getPosition(), 40.f, false)->Label().position);
			this->world->AddItem(item);
		}
		else if(!item->getVisible())
		{
			item->Reload(SGE::delta_time);
		}
	}
}
