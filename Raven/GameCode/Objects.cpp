#include "Objects.hpp"

Item::Item(b2Vec2 pos, Item::IType type): Object(pos, true, getCircle()), type(type)
{
	this->setLayer(0.5f);
	this->Item::setVisible(false);
}


void Item::useItem(RavenBot& bot)
{
	this->visible = false;
	this->cd = itemCD;
	this->consumeItem(bot);
}

HealthPack::HealthPack(): Item(b2Vec2_zero, IType::Health)
{}

HealthPack::HealthPack(b2Vec2 pos): Item(pos, IType::Health)
{}

void HealthPack::consumeItem(RavenBot& bot)
{
	bot.AddHealth(50.f);
}

ArmorPack::ArmorPack(): Item(b2Vec2_zero, IType::Armor)
{}
	
ArmorPack::ArmorPack(b2Vec2 pos): Item(pos, IType::Armor)
{}

void ArmorPack::consumeItem(RavenBot& bot)
{
	bot.AddArmor(50.f);
}

RailgunAmmo::RailgunAmmo(): Item(b2Vec2_zero, IType::RGAmmo)
{}

RailgunAmmo::RailgunAmmo(b2Vec2 pos): Item(pos, IType::RGAmmo)
{}

void RailgunAmmo::consumeItem(RavenBot& bot)
{
	bot.AddRailgunAmmo(10u);
}

RocketAmmo::RocketAmmo(): Item(b2Vec2_zero, IType::RLAmmo)
{}

RocketAmmo::RocketAmmo(b2Vec2 pos): Item(pos, IType::RLAmmo)
{}

void RocketAmmo::consumeItem(RavenBot& bot)
{
	bot.AddRocketAmmo(15u);
}
