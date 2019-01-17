#pragma once

#include <Object/sge_object.hpp>
#include "RavenBot.hpp"
#include "Utilities.hpp"

class Rocket: public SGE::Object
{
	constexpr static float width = 0.8f;
	constexpr static float height = 0.5f * width;
	constexpr static float speed = 5.f;
	constexpr static float radius = 4.f;
	static SGE::Shape* Shape()
	{
		static SGE::Shape* defShape = SGE::Shape::Rectangle(Width(), Height(), false);
		return defShape;
	}
	b2Vec2 heading = b2Vec2_zero;
	bool primed = false;
public:
	Rocket(b2Vec2 pos, b2Vec2 dir): Object(pos, true, Shape()), heading(dir)
	{
		this->Rocket::setVisible(true);
		this->setOrientation(this->heading.Orientation());
	}

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

	void Prime()
	{
		this->primed = true;
	}

	bool IsPrimed() const
	{
		return this->primed;
	}

	constexpr static float Width()
	{
		return  width;
	}
	
	constexpr static float Height()
	{
		return  height;
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

	Item(IType type);
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
		return !this->getVisible() && this->cd < 0.f;
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
	HealthPack();
};

class ArmorPack: public Item
{
protected:
	void consumeItem(RavenBot& bot) override;
public:
	ArmorPack();

};

class RailgunAmmo: public Item
{
protected:
	void consumeItem(RavenBot& bot) override;
public:
	RailgunAmmo();
};

class RocketAmmo: public Item
{
protected:
	void consumeItem(RavenBot& bot) override;
public:
	RocketAmmo();
};