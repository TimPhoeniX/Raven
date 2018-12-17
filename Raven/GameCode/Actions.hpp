#ifndef ZOMBIEGAME_ACTIONS
#define ZOMBIEGAME_ACTIONS

#include <Action/sge_action.hpp>
#include "Logics.hpp"

namespace SGE
{
	class Scene;
	class Object;
	class MouseObject;
	class Logic;
}

class Shoot : public SGE::Action
{
protected:
	Aim* shootLogic = nullptr;
public:
	explicit Shoot(Aim* shootLogic);

	void action_begin() override;

	void action_main() override;

	void action_ends() override;
};

class GOTO : public SGE::Action
{
	SGE::Object* object = nullptr;

public:
	GOTO(SGE::Object* object);
	void action_begin() override;
	void action_ends() override;
	void action_main() noexcept override;
};

class PortalAction : public SGE::Action
{
public:
	PortalAction();
	void action_begin() override;
	void action_main() override;
	void action_ends() override;
};

class LogicSwitch : public SGE::Action
{
	SGE::Logic* logic = nullptr;

public:
	LogicSwitch(SGE::Logic* id);
	void action_main() override;
	void action_ends() override;
	void action_begin() override;
};

class MouseClickedAction : public SGE::Action
{
	SGE::MouseObject* mouseObject = nullptr;
	SGE::Object* player = nullptr;

public:
	MouseClickedAction(SGE::MouseObject* mouseObject, SGE::Object* player);
	void action_main() noexcept override;
	void action_begin() override;
	void action_ends() override;
};

class Load : public SGE::Action
{
	SGE::Scene* nextScene = nullptr;
public:
	explicit Load(SGE::Scene* scene);
	void action_begin() override;
	void action_main() override;
	void action_ends() override;

};

#endif
