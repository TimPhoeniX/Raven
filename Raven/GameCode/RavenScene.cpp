#include <set>
#include <functional>

#include <boost/config/suffix.hpp>
#include <Logic/Logics/Camera/sge_logic_camera_zoom.hpp>
#include <Object/Shape/sge_shape.hpp>
#include <algorithm>
#include <random>

#include "RavenScene.hpp"
#include "Image.hpp"
#include "Logics.hpp"
#include "Actions.hpp"
#include "MovingObject.hpp"
#include "Utilities.hpp"
#include "PlayerMove.hpp"
#include "SteeringBehavioursUpdate.hpp"
#include "Game/InputHandler/sge_input_binder.hpp"
#include "Renderer/SpriteBatch/sge_sprite_batch.hpp"
#include "Renderer/sge_renderer.hpp"
#include <allocators>
#include "QuadBatch.hpp"

bool RavenScene::init()
{
	return true;
}

constexpr float Width = 100.f;
constexpr float Height = 100.f;
constexpr size_t ObstaclesNum = 10u;

RavenScene::RavenScene(SGE::Game* game, const char* path): Scene(), world(Width, Height), game(game),
path([game](const char* path)
{
	return game->getGamePath() + path;
}(path))
{
	static bool initialized = init();
}

void RavenScene::loadScene()
{
	//RenderBatches
	SGE::BatchRenderer* renderer = SGE::Game::getGame()->getRenderer();
	this->level = SGE::Level();
	GLuint basicProgram = renderer->getProgramID("BatchShader.vert", "BatchShader.frag");
	GLuint scaleUVProgram = renderer->getProgramID("BatchUVShader.vert", "BatchShader.frag");
	GLuint QuadProgram = renderer->getProgramID("QuadBatchShader.vert", "BatchShader.frag");

	std::string lightBrickTexPath = "Resources/Textures/light_bricks.png";
	std::string zombieTexPath = "Resources/Textures/zombie.png";
	std::string beamPath = "Resources/Textures/pointer.png";
	//Change
	std::string rocketPath = "Resources/Textures/pointer.png";

	SGE::RealSpriteBatch* wallBatch = renderer->getBatch(renderer->newBatch(scaleUVProgram, lightBrickTexPath, 4, false, true));
	SGE::RealSpriteBatch* obstacleBatch = renderer->getBatch(renderer->newBatch<QuadBatch>(QuadProgram, lightBrickTexPath, 10, false, true));
	SGE::RealSpriteBatch* beamBatch = renderer->getBatch(renderer->newBatch(basicProgram, beamPath, 5));
	SGE::RealSpriteBatch* rocketBatch = renderer->getBatch(renderer->newBatch(basicProgram, beamPath, 10));
	SGE::RealSpriteBatch* botBatch = renderer->getBatch(renderer->newBatch(basicProgram, zombieTexPath, 5));

	GLuint IBO = botBatch->initializeIBO();
	GLuint sampler = botBatch->initializeSampler();

	for(SGE::RealSpriteBatch* b : {wallBatch,beamBatch,obstacleBatch, rocketBatch})
	{
		b->initializeIBO(IBO);
		b->initializeSampler(sampler);
	}
	//RenderBatches

	auto& world = this->level.getWorld();
	world.reserve(4 + ObstaclesNum);

	//Boundaries
	SGE::Shape* horizontal = SGE::Shape::Rectangle(Width, 1.f, false);
	SGE::Shape* vertical = SGE::Shape::Rectangle(1.f, Height + 2.f, false);

	world.emplace_back(-0.5f, Height * 0.5f, lightBrickTexPath);
	world.back().setShape(vertical);
	this->world.AddWall(&world.back(), Wall::Right);
	wallBatch->addObject(&world.back());

	world.emplace_back(Width + .5f, Height * 0.5f, lightBrickTexPath);
	world.back().setShape(vertical);
	this->world.AddWall(&world.back(), Wall::Left);
	wallBatch->addObject(&world.back());

	world.emplace_back(Width * 0.5f, Height + .5f, lightBrickTexPath);
	world.back().setShape(horizontal);
	this->world.AddWall(&world.back(), Wall::Bottom);
	wallBatch->addObject(&world.back());

	world.emplace_back(Width * 0.5f, -0.5f, lightBrickTexPath);
	world.back().setShape(horizontal);
	this->world.AddWall(&world.back(), Wall::Top);
	wallBatch->addObject(&world.back());
	//Boundaries

}

void RavenScene::unloadScene()
{
	this->finalize();
}

RavenScene::~RavenScene()
{
	std::cout << "~MainScene" << std::endl;
}

template<typename Vec>
void vec_clear(Vec& vec)
{
	for(auto h : vec)
	{
		delete h;
	}
	vec.clear();
}

void RavenScene::finalize()
{
	game->getRenderer()->deleteSceneBatch(this);
	this->level.clear();
	this->world.clear();
	vec_clear(this->getLogics());
	vec_clear(this->getActions());
	this->getObjects().clear();
	game->unmapAll();
}

void RavenScene::onDraw()
{}
