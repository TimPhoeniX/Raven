#pragma once

#include <Object/sge_object.hpp>
#include "RavenBot.hpp"
#include "utility"
#include "Utilities.hpp"

class Rocket: public SGE::Object
{
	constexpr static float Width = 0.6f;
	constexpr static float Height = 0.5f * Width;
	constexpr static float speed = 3.f;
	constexpr static float radius = 4.f;
	static SGE::Shape* getShape()
	{
		static SGE::Shape* defShape = SGE::Shape::Rectangle(Width, Height, false);
		return defShape;
	}
	b2Vec2 heading = b2Vec2_zero;
public:
	Rocket(b2Vec2 pos, b2Vec2 dir): Object(pos, true, getShape()), heading(dir)
	{}

	float Speed() const
	{
		return this->speed;
	}

	float Radius() const
	{
		return this->radius;
	}

	b2Vec2 Heading() const
	{
		return this->heading;
	}
};

class Item: public SGE::Object
{
public:
	Item(b2Vec2 pos);
	virtual void useItem(RavenBot*) = 0;
	void Reset()
	{
		this->visible = true;
	}
};

class HealthPack: public Item
{
public:
	HealthPack(b2Vec2 pos);
	void useItem(RavenBot* bot) override;
};

class ArmorPack: public Item
{
public:
	ArmorPack(b2Vec2 pos);

	void useItem(RavenBot* bot) override;
};

class RailgunAmmo: public Item
{
public:
	RailgunAmmo(b2Vec2 pos);

	void useItem(RavenBot* bot) override;
};

class RocketAmmo: public Item
{
public:
	RocketAmmo(b2Vec2 pos);

	void useItem(RavenBot* bot) override;
};