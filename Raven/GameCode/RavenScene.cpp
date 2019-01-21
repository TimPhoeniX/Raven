#include <set>
#include <functional>

#include <Logic/Logics/Camera/sge_logic_camera_zoom.hpp>
#include <Object/Shape/sge_shape.hpp>
#include <algorithm>
#include <random>

#include "RavenScene.hpp"
#include "Image.hpp"
#include "Logics.hpp"
#include "Actions.hpp"
#include "RavenBot.hpp"
#include "Utilities.hpp"
#include "SteeringBehavioursUpdate.hpp"
#include "Game/InputHandler/sge_input_binder.hpp"
#include "Renderer/SpriteBatch/sge_sprite_batch.hpp"
#include "Renderer/sge_renderer.hpp"
#include <allocators>
#include "QuadBatch.hpp"
#include "QuadObject.hpp"
#include <queue>
#include "Graph.hpp"

class RGTrace: public SGE::Object
{
public:
	RGTrace(): Object(b2Vec2_zero, true)
	{
		this->Object::setVisible(false);
	}
	~RGTrace() = default;
};

class Distance
{
public:
	float operator()(const GridVertex* cur, const GridVertex* end) const
	{
		return b2Distance(cur->Label().position, end->Label().position);
	}
};

class DiagonalDistance
{
	const static float sqrt2;
public:
	float operator()(const GridVertex* cur, const GridVertex* end) const
	{
		auto node = cur->Label().position, goal = end->Label().position;
		float dx = abs(node.x - goal.x), dy = abs(node.y - goal.y);
		return dx < dy ? dx * sqrt2 + dy - dx : dy * sqrt2 + dx - dy;
	}
};
const float DiagonalDistance::sqrt2 = sqrt(2.f);


void RavenGameState::InitRandomEngine()
{
	this->rand = std::bind(std::uniform_int_distribution<size_t>(0, graph.VertexCount()-1), std::default_random_engine{});
}

GridCell* RavenGameState::GetCell(b2Vec2 pos)
{
	size_t x = size_t(std::floor(pos.x)), y = size_t(std::floor(pos.y));
	return &(cells[y][x]);
}

GridVertex* RavenGameState::GetVertex(b2Vec2 pos)
{
	GridVertex* res = this->GetCell(pos)->vertex;
	if(!res)
	{
		b2Vec2 dir = {-0.5f, 0.f};
		while(!res)
		{
			res = this->GetCell(pos + dir)->vertex;
			dir = b2Mul(b2Rot(0.25f * b2_pi), dir);
		}
	}
	return res;
}

GridVertex* RavenGameState::GetRandomVertex(const b2Vec2& position, const float limit)
{
	GridVertex* res = graph[this->rand()];
	while(b2DistanceSquared(position, res->Label().position) < limit * limit)
	{
		res = graph[this->rand()];
	}
	return res;
}

Path RavenGameState::GetPath(GridVertex * begin, GridVertex * end)
{
	this->graph.AStar(begin, end, DiagonalDistance());
	return Path(begin, end);
}

void RavenGameState::UseItem(Item* item)
{
	for(auto& bot: this->bots)
	{
		bot.items.erase(item);
	}
	this->world->RemoveItem(item);
}

void RavenGameState::NewRocket(b2Vec2 pos, b2Vec2 direction)
{
	direction.Normalize();
	Rocket* rocket = new Rocket(pos + 0.5f * direction, direction);
	this->world->AddObstacle(rocket);
	this->rocketBatch->addObject(rocket);
	this->rockets.push_back(rocket);
}

void RavenGameState::RemoveRocket(Rocket* rocket)
{
	this->world->RemoveObstacle(rocket);
	this->rocketBatch->removeObject(rocket);
	this->rockets.erase(std::find(this->rockets.begin(), this->rockets.end(), rocket));
	delete rocket;
}

template <typename T>
void RavenGameState::GenerateItems(const size_t bots, SGE::RealSpriteBatch* batch)
{
	for(size_t i = 0u; i < bots; ++i)
	{
		Item* item = new T();
		batch->addObject(item);
		this->items.push_back(item);
	}
}

bool RavenScene::init()
{
	return true;
}

constexpr size_t ObstaclesNum = 10u;

RavenScene::RavenScene(SGE::Game* game, const char* path) : Scene(), world(Width, Height), game(game),
path([game](const char* path)
{
	return game->getGamePath() + path;
}(path))
{
	static bool initialized = init();
}

struct GridCellBuild
{
	size_t x = 0u, y = 0u;
	enum State
	{
		Accepted,
		Queued,
		Untested,
		Invalid
	} state = State::Untested;
	SGE::Object* dummy = nullptr;
	GridVertex* vertex;
};

class GraphCellDummy: public SGE::Object
{
public:
	GraphCellDummy(size_t x, size_t y) : Object(0.5f + x, 0.5f + y, true, getCircle()){}
	GraphCellDummy(b2Vec2 pos) : Object(pos.x, pos.y, true, getCircle()) {}
};

class GraphCellDummy1: public GraphCellDummy
{
public:
	GraphCellDummy1(size_t x, size_t y): GraphCellDummy(x,y)
	{
		this->Object::setShape(SGE::Shape::Circle(0.3f, true));
	}
	GraphCellDummy1(b2Vec2 pos): GraphCellDummy(pos)
	{
		this->Object::setShape(SGE::Shape::Circle(0.3f, true));
	}
};

class GraphEdgeDummy: public SGE::Object
{
public:
	GraphEdgeDummy(size_t x, size_t y): Object(0.5f + x, 0.5f + y, true, getCircle())
	{}
	GraphEdgeDummy(b2Vec2 pos): Object(pos.x, pos.y, true, getCircle())
	{}
};

constexpr size_t Bots = 5u;

void RavenScene::loadScene()
{
	this->gs = new RavenGameState();
	this->gs->world = &this->world;

	//RenderBatches
	SGE::BatchRenderer* renderer = SGE::Game::getGame()->getRenderer();
	this->level = SGE::Level();
	GLuint basicProgram = renderer->getProgramID("BatchShader.vert", "BatchShader.frag");
	GLuint scaleUVProgram = renderer->getProgramID("BatchUVShader.vert", "BatchShader.frag");
	GLuint QuadProgram = renderer->getProgramID("QuadBatchShader.vert", "BatchShader.frag");

	std::string lightBrickTexPath = "Resources/Textures/light_bricks.png";
	std::string zombieTexPath = "Resources/Textures/zombie.png";
	std::string cellTexPath = "Resources/Textures/cell.png";
	std::string beamPath = "Resources/Textures/pointer.png";
	std::string rocketPath = "Resources/Textures/rocket.png";
	std::string healthPath = "Resources/Textures/health.png";
	std::string armorPath = "Resources/Textures/armor.png";
	std::string rlammoPath = "Resources/Textures/rlammo.png";
	std::string rgammoPath = "Resources/Textures/rgammo.png";

	SGE::RealSpriteBatch* wallBatch = renderer->getBatch(renderer->newBatch(scaleUVProgram, lightBrickTexPath, 4, false, true));
	SGE::RealSpriteBatch* obstacleBatch = renderer->getBatch(renderer->newBatch<QuadBatch>(QuadProgram, lightBrickTexPath, 10, false, true));
	SGE::RealSpriteBatch* botBatch = renderer->getBatch(renderer->newBatch(basicProgram, zombieTexPath, Bots));

	this->gs->railBatch = renderer->getBatch(renderer->newBatch(basicProgram, beamPath, Bots));
	this->gs->rocketBatch = renderer->getBatch(renderer->newBatch(basicProgram, rocketPath, Bots * 20u));

	SGE::RealSpriteBatch* healthBatch = renderer->getBatch(renderer->newBatch(basicProgram, healthPath, Bots));
	SGE::RealSpriteBatch* armorBatch = renderer->getBatch(renderer->newBatch(basicProgram, armorPath, Bots));
	SGE::RealSpriteBatch* rgammoBatch = renderer->getBatch(renderer->newBatch(basicProgram, rgammoPath, Bots));
	SGE::RealSpriteBatch* rlammoBatch = renderer->getBatch(renderer->newBatch(basicProgram, rlammoPath, Bots));

	SGE::RealSpriteBatch* graphTestBatch = renderer->getBatch(renderer->newBatch(basicProgram, cellTexPath, size_t(Width * Height), false, true));
	SGE::RealSpriteBatch* graphEdgeTestBatch = renderer->getBatch(renderer->newBatch(basicProgram, "Resources/Textures/path.png", size_t(Width * Height * 8u), false, true));

	QuadBatch* obBatch = dynamic_cast<QuadBatch*>(obstacleBatch);
	if (!obBatch)
		throw std::runtime_error("QuadBatch cast failed!");

	GLuint IBO = botBatch->initializeIBO();
	GLuint sampler = botBatch->initializeSampler();
	obstacleBatch->initializeIBO(IBO);
	obstacleBatch->initializeSampler(GL_REPEAT, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

	for (SGE::RealSpriteBatch* b : { wallBatch, this->gs->railBatch, this->gs->rocketBatch, healthBatch, armorBatch, rgammoBatch, rlammoBatch })
	{
		b->initializeIBO(IBO);
		b->initializeSampler(sampler);
	}
	//!RenderBatches

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
	//!Boundaries

	//Camera
	SGE::Camera2d* cam = game->getCamera();
	cam->setPosition({ 32.f * Width, 32.f * Height });
	cam->setCameraScale(0.197f);
	this->addLogic(new SpectatorCamera(10, SGE::Key::W, SGE::Key::S, SGE::Key::A, SGE::Key::D, cam));
	this->addLogic(new SGE::Logics::CameraZoom(cam, 0.5f, 1.f, 0.197f, SGE::Key::Q, SGE::Key::E));
	//!Camera

	//Obstacles
	using Quad = QuadBatch::Quad;
	std::uniform_real_distribution<float> angle_distribution(-b2_pi, b2_pi);
	std::mt19937 engine((std::random_device{})());
	auto angle = std::bind(angle_distribution, engine);
	Quad q1 = { 4.f * glm::vec2{-64.f, -64.f}, 4.f * glm::vec2{300.f, -300.f}, 4.f * glm::vec2{128.f, 400.f}, 4.f * glm::vec2{ -80.f, 128.f } };
	SGE::Object* obstacle1 = new QuadObstacle(Width * .5f, Height * .5f, angle(), q1);
	SGE::Object* obstacle2 = new QuadObstacle(Width * .5f + 10.f, Height * .5f, angle(), q1);
	obBatch->addObject(obstacle1, q1);
	obBatch->addObject(obstacle2, q1);
	this->world.AddObstacle(obstacle1);
	this->world.AddObstacle(obstacle2);
	
	this->gs->obstacles.assign({obstacle1, obstacle2});

	//Grid
//#define GraphCellDebug
//#define GraphEdgeDebug
	{
		std::queue<GridCellBuild*> cells;
		int intersections = 0;
		GridCellBuild grid[Y][X];
		std::pair<int, int> directions[8] =
		{
			{-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}
		};
		b2Vec2 points[4] = {{-.5f, 0.f}, {0.f, -.5f}, {0.5f, 0.f}, {0.f, .5f}};
		b2Vec2 diags[4];
		for(size_t i = 0; i < 4u; ++i)
			diags[i] = b2Mul(b2Rot(0.5f * b2_pi), points[i]);

		for(size_t x = 0u; x < X; ++x)
			for(size_t y = 0u; y < Y; ++y)
			{
				grid[y][x].x = x;
				grid[y][x].y = y;
			}

		grid[0][0].state = GridCellBuild::Queued;
		cells.push(&grid[0][0]);

		while(!cells.empty())
		{
			GridCellBuild& currentCell = *cells.front();
			cells.pop();
			intersections = 0;
			b2Vec2 pos = b2Vec2{0.5f + currentCell.x, 0.5f + currentCell.y};
			std::vector<SGE::Object*> obstacles = std::move(this->world.getObstacles(pos, 1.5f));
			for(SGE::Object* o : obstacles)
			{
				QuadObstacle* qo = dynamic_cast<QuadObstacle*>(o);
				if(!qo) continue;
				for(Edge edge : qo->getEdges())
				{
					b2Vec2 radius = -edge.Normal();
					radius *= 0.5f;
					float dist = b2DistanceSquared(pos, edge.From());
					if(dist <= 0.25f)
					{
						currentCell.state = GridCellBuild::Invalid;
						break;
					}
					b2Vec2 intersection;
					if(LineIntersection(pos, pos + radius, edge.From(), edge.To(), dist, intersection))
					{
						currentCell.state = GridCellBuild::Invalid;
						break;
					}
					if(LineIntersection(pos, pos + b2Vec2{100.f, 0.f}, edge.From(), edge.To(), dist, intersection))
					{
						++intersections;
					}
				}
				if(currentCell.state == GridCellBuild::Invalid || 1 == intersections % 2)
					break;
			}
			if(currentCell.state != GridCellBuild::Invalid && 0 == intersections % 2)
			{
				currentCell.state = GridCellBuild::Accepted;
				currentCell.vertex = new GridVertex(CellLabel(pos));
				this->gs->graph.AddVertex(currentCell.vertex);
#ifdef GraphCellDebug
				currentCell.dummy = new GraphCellDummy(currentCell.x, currentCell.y);
				graphTestBatch->addObject(currentCell.dummy);
#endif
				for(size_t i = 0u; i < 8u; ++i)
				{
					auto dir = directions[i];
					size_t x = currentCell.x + dir.first;
					size_t y = currentCell.y + dir.second;
					b2Vec2 edgeVec = b2Vec2{float(dir.first), float(dir.second)};
					if(x < X && y < Y)
					{
						GridCellBuild& otherCell = grid[y][x];
						switch(otherCell.state)
						{
						case GridCellBuild::Accepted:
						{
							bool intersected = false;
							for(SGE::Object* o : obstacles)
							{
								QuadObstacle* qo = dynamic_cast<QuadObstacle*>(o);
								if(!qo) continue;
								for(Edge edge : qo->getEdges())
								{
									b2Vec2(&pts)[4] = i % 2 ? diags : points;
									for(b2Vec2 offset : pts)
									{
										b2Vec2 from = pos + offset;
										b2Vec2 to = from + edgeVec;
										float dist;
										b2Vec2 inters;
										intersected = LineIntersection(from, to, edge.From(), edge.To(), dist, inters);
										if(intersected)
										{
											break;
										}
									}
									if(intersected) break;
								}
								if(intersected) break;
							}
							if(!intersected)
							{
								this->gs->graph.AddEdge(currentCell.vertex, otherCell.vertex, edgeVec.Length());
#ifdef GraphEdgeDebug
								auto edgeOb = new GraphEdgeDummy(pos + 0.5f * edgeVec);
								edgeOb->setOrientation(edgeVec.Orientation());
								edgeOb->setLayer(.5f);
								edgeOb->setShape(SGE::Shape::Rectangle(edgeVec.Length(), 0.05f, true));
								graphEdgeTestBatch->addObject(edgeOb);
#endif
							}
							break;
						}
						case GridCellBuild::Queued: break;
						case GridCellBuild::Untested:
						{
							otherCell.state = GridCellBuild::Queued;
							cells.push(&otherCell);
							break;
						}
						case GridCellBuild::Invalid: break;
						default: break;
						}
					}
				}
			}
		}
		for(size_t x = 0u; x < X; ++x)
		{
			for(size_t y = 0u; y < Y; ++y)
			{
				if(grid[y][x].state == GridCellBuild::Accepted)
				{
					auto& cell = this->gs->cells[y][x];
					cell.state = GridCell::Valid;
					cell.vertex = grid[y][x].vertex;
				}
			}
		}
//#define ASTARDEBUG
#ifdef ASTARDEBUG
		//Test
		GridVertex* begin = this->gs->cells[0][0].vertex;
		GridVertex* end = this->gs->cells[Y-1u][X - 1u].vertex;
		this->gs->graph.AStar(begin, end, DiagonalDistance{});
		while(end != begin)
		{
			graphTestBatch->addObject(new GraphCellDummy(end->Label().position));
			end = end->Parent();
		}
		for(auto v : this->gs->graph)
		{
			if(v->State() != CTL::VertexState::White)
			{
				graphTestBatch->addObject(new GraphCellDummy1(v->Label().position));
			}
		}
#endif
	} //!Grid

	//Spawn Points
	std::uniform_int_distribution<size_t> randWidth(3u, X/4u -3u);
	std::uniform_int_distribution<size_t> randHeight(3u, Y/4u - 3u);
	auto randW = std::bind(randWidth, engine);
	auto randH = std::bind(randHeight, engine);
	for(size_t xI = 0u; xI < 4u; ++xI)
	{
		for(size_t yI = 0u; yI < 4u; ++yI)
		{
			size_t x = xI * (X/4u), y = yI * (Y/4u);
			GridCell* cell = nullptr;
			int tries = 30;
			while(!cell && tries)
			{
				--tries;
				cell = &(this->gs->cells[y + randH()][x + randW()]);
				if(cell->state == GridCell::Invalid)
				{
					cell = nullptr;
				}
			}
			if(!tries)
			{
				std::cout << "No cells at" << xI << ' ' << yI << std::endl;
				continue;
			}
			this->gs->spawnPoints.push_back(cell);
		}
	}
	for(auto cell : this->gs->spawnPoints)
	{
		//graphTestBatch->addObject(new GraphCellDummy(cell->vertex->Label().position));
	}
	//Players
	{
		this->gs->bots.reserve(Bots);
		decltype(this->gs->spawnPoints) pts = this->gs->spawnPoints;
		std::random_shuffle(pts.begin(), pts.end());
		for(int i = 0; i < Bots; ++i)
		{
			this->gs->bots.emplace_back(pts[i]->vertex->Label().position, getCircle(), &this->world);
			RavenBot* bot = &this->gs->bots.back();
			botBatch->addObject(bot);
			this->world.AddMover(bot);
			bot->RailgunTrace = new RGTrace();
			this->gs->railBatch->addObject(bot->RailgunTrace);
		}
	}
	{
		this->gs->GenerateItems<HealthPack>(Bots, healthBatch);
		this->gs->GenerateItems<ArmorPack>(Bots, armorBatch);
		this->gs->GenerateItems<RocketAmmo>(Bots, rlammoBatch);
		this->gs->GenerateItems<RailgunAmmo>(Bots, rgammoBatch);
	}
	
	
	this->gs->InitRandomEngine();

	//Logics
	this->addLogic(new SteeringBehavioursUpdate(&this->gs->bots));
	this->addLogic(new SeparateBots(&this->world, &this->gs->bots));
	this->addLogic(new MoveAwayFromObstacle(&this->world, this->gs->obstacles));
	this->addLogic(new MoveAwayFromWall(&this->world, this->gs->bots));
	this->addLogic(new BotLogic(&this->world, this->gs));
	this->addLogic(new ItemLogic(&this->world, this->gs));
	this->addLogic(new RocketLogic(this->gs, &this->world));
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
	delete this->gs;
	this->level.clear();
	this->world.clear();
	vec_clear(this->getLogics());
	vec_clear(this->getActions());
	this->getObjects().clear();
	game->unmapAll();
}

void RavenScene::onDraw()
{}
