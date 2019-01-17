#include "Objects.hpp"

Item::Item(Item::IType type): Object(b2Vec2_zero, true, getCircle()), type(type)
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

HealthPack::HealthPack(): Item(IType::Health)
{
}

void HealthPack::consumeItem(RavenBot& bot)
{
	bot.AddHealth(50.f);
}

ArmorPack::ArmorPack(): Item(IType::Armor)
{
}

void ArmorPack::consumeItem(RavenBot& bot)
{
	bot.AddArmor(50.f);
}

RailgunAmmo::RailgunAmmo(): Item(IType::RGAmmo)
{
}

void RailgunAmmo::consumeItem(RavenBot& bot)
{
	bot.AddRailgunAmmo(10u);
}

RocketAmmo::RocketAmmo(): Item(IType::RLAmmo)
{
}

void RocketAmmo::consumeItem(RavenBot& bot)
{
	bot.AddRocketAmmo(15u);
}
