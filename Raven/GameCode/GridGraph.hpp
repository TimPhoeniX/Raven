#pragma once
#include "Graph.hpp"
#include "Box2D/Common/b2Math.h"

struct CellLabel
{
	b2Vec2 position = b2Vec2_zero;
	CellLabel() = default;
	explicit CellLabel(b2Vec2 pos): position(pos)
	{}
	~CellLabel() = default;
};

using GridGraph = CTL::Graph<CellLabel>;
using GridVertex = GridGraph::Vertex;