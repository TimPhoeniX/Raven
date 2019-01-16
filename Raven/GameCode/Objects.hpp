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

	constexpr static float Speed()
	{
		return speed;
	}

	constexpr static float Radius()
	{
		return radius;
	}

	b2Vec2 Heading() const
	{
		return this->heading;
	}
};

class Item: public SGE::Object
{
public:
	enum class IType: unsigned
	{
		Health, Armor, RLAmmo, RGAmmo
	};
protected:
	constexpr static float itemCD = 15.f;
	virtual void consumeItem(RavenBot&) = 0;
	IType type;
	float cd = itemCD;

	Item(b2Vec2 pos, IType type);
public:
	void useItem(RavenBot&);


	IType Type() const
	{
		return this->type;
	}

	void Reload(float delta)
	{
		if(this->cd > 0.f) this->cd -= delta;
	}

	bool Respawnable() const
	{
		return this->cd < 0.f;
	}

	void Respawn(b2Vec2 position)
	{
		this->setVisible(true);
		this->setPosition(position);
	}
};

class HealthPack: public Item
{
protected:
	void consumeItem(RavenBot& bot) override;
public:
	HealthPack(b2Vec2 pos);
};

class ArmorPack: public Item
{
protected:
	void consumeItem(RavenBot& bot) override;
public:
	ArmorPack(b2Vec2 pos);

};

class RailgunAmmo: public Item
{
protected:
	void consumeItem(RavenBot& bot) override;
public:
	RailgunAmmo(b2Vec2 pos);
};

class RocketAmmo: public Item
{
protected:
	void consumeItem(RavenBot& bot) override;
public:
	RocketAmmo(b2Vec2 pos);
};