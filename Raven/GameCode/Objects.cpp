#include "Objects.hpp"

Item::Item(b2Vec2 pos): Object(pos, true, getCircle())
{
}

HealthPack::HealthPack(b2Vec2 pos): Item(pos)
{
}

void HealthPack::useItem(RavenBot* bot)
{
	this->visible = false;
	bot->AddHealth(50.f);
}

ArmorPack::ArmorPack(b2Vec2 pos): Item(pos)
{
}

void ArmorPack::useItem(RavenBot* bot)
{
	this->visible = false;
	bot->AddArmor(50.f);
}

RailgunAmmo::RailgunAmmo(b2Vec2 pos): Item(pos)
{
}

void RailgunAmmo::useItem(RavenBot* bot)
{
	this->visible = false;
	bot->AddRailgunAmmo(10u);
}

RocketAmmo::RocketAmmo(b2Vec2 pos): Item(pos)
{
}

void RocketAmmo::useItem(RavenBot* bot)
{
	this->visible = false;
	bot->AddRocketAmmo(15u);
}
