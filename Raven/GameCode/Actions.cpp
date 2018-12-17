#include "Actions.hpp"
#include <iostream>

#include <Object/Camera2d/sge_camera2d.hpp>
#include <Game/Director/sge_director.hpp>
#include "Game/sge_game.hpp"
#include "IO/Mouse/sge_mouse.hpp"

Shoot::Shoot(Aim* shootLogic): Action(true), shootLogic(shootLogic)
{
}

void Shoot::action_begin()
{
}

void Shoot::action_main()
{
	this->shootLogic->Shoot();
}

void Shoot::action_ends()
{
}

GOTO::GOTO(SGE::Object* object): Action(false), object(object)
{
}

void GOTO::action_main() noexcept
{
	this->object->setPositionGLM(200, 200);
	//NOTE: we don't want this action to be active forever
}

PortalAction::PortalAction(): SGE::Action(false)
{
}

void PortalAction::action_begin()
{
	std::cout << "portal::begin" << std::endl;
}

void PortalAction::action_main()
{
	std::cout << "portal::main" << std::endl;
	//NOTE: we DON'T want this action to be active forever -- only when the logic condition form Portal is met
}

void PortalAction::action_ends()
{
	std::cout << "portal::end" << std::endl;
}

LogicSwitch::LogicSwitch(SGE::Logic* id): SGE::Action(true), logic(id)
{
}

void LogicSwitch::action_main()
{
	logic->toggleOn();
	//NOTE: we want this action to be active forever
}


MouseClickedAction::MouseClickedAction(SGE::MouseObject* mouseObject, SGE::Object* player): Action(true), mouseObject(mouseObject), player(player)
{
}

void MouseClickedAction::action_main() noexcept
{
	glm::vec2 coords = this->mouseObject->getMouseCoords();
	glm::vec2 worldCoords = SGE::Game::getGame()->getCamera()->screenToWorld(coords);
	std::cout << "[Clicked] - x: " << worldCoords.x << ", y: " << worldCoords.y << '\n'; //No 
	std::cout << "[Player ] - x: " << this->player->getXGLM() << ", y: " << this->player->getYGLM() << std::endl;

	//NOTE: we want this action to be active forever
}

Load::Load(SGE::Scene* scene) : nextScene(scene)
{
}

void Load::action_begin()
{
}

void Load::action_main()
{
	auto d = SGE::Director::getDirector();
	d->toNextScene(this->nextScene);
	this->active = false;
}

void Load::action_ends()
{
}

//TODO Make them normal virtual?
void GOTO::action_begin()
{
}

void GOTO::action_ends()
{
}

void LogicSwitch::action_ends()
{
}

void LogicSwitch::action_begin()
{
}

void MouseClickedAction::action_begin()
{
}

void MouseClickedAction::action_ends()
{
}