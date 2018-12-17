#pragma once
#include <Logic/sge_logic.hpp>
#include <vector>
#include "MovingObject.hpp"

class SteeringBehavioursUpdate: public SGE::Logic
{
protected:
	std::vector<MovingObject>* objects = nullptr;
public:
	explicit SteeringBehavioursUpdate(std::vector<MovingObject>* objects);
	virtual void performLogic() override;
};
