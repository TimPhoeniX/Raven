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
#include "QuadObject.hpp"

bool RavenScene::init()
{
	return true;
}

constexpr float Width = 80.f;
constexpr float Height = 60.f;
constexpr size_t ObstaclesNum = 10u;

RavenScene::RavenScene(SGE::Game* game, const char* path) : Scene(), world(Width, Height), game(game),
path([game](const char* path)
{
	return game->getGamePath() + path;
}(path))
{
	static bool initialized = init();
}

struct GridCell
{
	enum State
	{
		Accepted,
		Untested,
		Invalid
	};
	State state = State::Untested;
	SGE::Object* dummy = nullptr;
};

class GraphCellDummy: public SGE::Object
{
public:
	GraphCellDummy(size_t x, size_t y) : Object(0.5f + x, 0.5f + y, true, getCircle()){}
	GraphCellDummy(b2Vec2 pos) : Object(pos.x, pos.y, true, getCircle()) {}
};

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
	SGE::RealSpriteBatch* graphTestBatch = renderer->getBatch(renderer->newBatch(basicProgram, zombieTexPath, size_t(Width*Height)));
	QuadBatch* obBatch = dynamic_cast<QuadBatch*>(obstacleBatch);
	if (!obBatch)
		throw std::runtime_error("QuadBatch cast failed!");

	GLuint IBO = botBatch->initializeIBO();
	GLuint sampler = botBatch->initializeSampler();
	obstacleBatch->initializeIBO(IBO);
	obstacleBatch->initializeSampler(GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

	for (SGE::RealSpriteBatch* b : { wallBatch, beamBatch, rocketBatch })
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

	//Camera
	SGE::Camera2d* cam = game->getCamera();
	cam->setPosition({ 32.f * Width, 32.f * Height });
	cam->setCameraScale(0.18f);
	this->addLogic(new SpectatorCamera(10, SGE::Key::W, SGE::Key::S, SGE::Key::A, SGE::Key::D, cam));
	this->addLogic(new SGE::Logics::CameraZoom(cam, 0.5f, 1.f, 0.178f, SGE::Key::Q, SGE::Key::E));
	//Camera

	//Obstacles
	using Quad = QuadBatch::Quad;
	Quad q1 = { 4.f * glm::vec2{-64.f, -64.f}, 4.f * glm::vec2{300.f, -300.f}, 4.f * glm::vec2{128.f, 400.f}, 4.f * glm::vec2{ -80.f, 128.f } };
	SGE::Object* obstacle1 = new QuadObstacle(Width * .5f, Height * .5f, 0.f, q1);
	SGE::Object* obstacle2 = new QuadObstacle(Width * .5f + 10.f, Height * .5f, b2_pi*0.5f, q1);
	obBatch->addObject(obstacle1, q1);
	obBatch->addObject(obstacle2, q1);
	this->world.AddObstacle(obstacle1);
	this->world.AddObstacle(obstacle2);

	//Grid
	constexpr size_t X = size_t(Width);
	constexpr size_t Y = size_t(Height);
	std::vector<SGE::Object*> obstacles;
	size_t x = 0u, y = 0u;
	GridCell::State gs = GridCell::Accepted;
	int intersections = 0;
	for(x = 0u; x < X; ++x)
	{
		for(y = 0u; y < Y; ++y)
		{
			b2Vec2 pos = b2Vec2{ 0.5f + x, 0.5f + y };
			obstacles = std::move(this->world.getObstacles(pos, 1.f));
			for(SGE::Object* o : obstacles)
			{
				QuadObstacle* qo = dynamic_cast<QuadObstacle*>(o);
				if (!qo) continue;
				for(Edge edge : qo->getEdges())
				{
					b2Vec2 radius = -edge.Normal();
					radius *= 0.5f;
					float dist;
					b2Vec2 intersection;
					if(LineIntersection(pos, pos + radius, edge.To(), edge.From(), dist, intersection))
					{
						gs = GridCell::Invalid;
						break;
					}
					if(LineIntersection(pos, pos + b2Vec2{100.f, 0.f}, edge.To(), edge.From(), dist, intersection))
					{
						++intersections;
					}
				}
				if (gs == GridCell::Invalid && 1 == intersections % 2)
					break;
			}
			auto stateToString = [](GridCell::State s)-> std::string
			{
				switch (s)
				{
				case GridCell::Accepted: return "Accepted";
				case GridCell::Untested: return "Untested";
				case GridCell::Invalid: return "Invalid";
				default: return "NAS";
				}
			};
			if(gs == GridCell::Accepted && 0 == intersections % 2)
			{
				graphTestBatch->addObject(new GraphCellDummy(x, y));
			}
			else
			{
				std::cout << "Failed X: " << x << ':' << y << ' ' << stateToString(gs) << " intersections: " << intersections << std::endl;
			}
			gs = GridCell::Accepted;
			intersections = 0;
		}
	}
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
	for (auto h : vec)
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
