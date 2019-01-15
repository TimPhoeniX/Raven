#pragma once
#include <Logic/sge_logic.hpp>
#include <vector>
#include "RavenBot.hpp"

class SteeringBehavioursUpdate: public SGE::Logic
{
protected:
	std::vector<RavenBot>* objects = nullptr;
public:
	explicit SteeringBehavioursUpdate(std::vector<RavenBot>* objects);
	virtual void performLogic() override;
};
