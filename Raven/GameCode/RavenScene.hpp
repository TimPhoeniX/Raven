#ifndef RAVEN_SCENE
#define RAVEN_SCENE

#include <Game/sge_game.hpp>
#include <Scene/sge_scene.hpp>
#include "RavenBot.hpp"
#include "World.hpp"
#include "GridGraph.hpp"
#include "Objects.hpp"
#include "Actions.hpp"
#include "Actions.hpp"

namespace SGE
{
	class RealSpriteBatch;
}

struct GridCell
{
	enum State
	{
		Valid, Invalid
	} state = Invalid;
	SGE::Object* object = nullptr;
	GridVertex* vertex = nullptr;
};

constexpr float Width = 80.f;
constexpr float Height = 60.f;
constexpr size_t X = size_t(Width);
constexpr size_t Y = size_t(Height);

class RavenGameState
{
protected:
	std::function<size_t()> rand;
public:
	GridCell cells[Y][X];
	GridGraph graph;
	std::vector<GridCell*> spawnPoints;
	std::vector<RavenBot> bots;
	std::vector<Rocket*> rockets;
	SGE::RealSpriteBatch* railBatch;
	SGE::RealSpriteBatch* rocketBatch;
	std::vector<SGE::Object*> obstacles;

	void InitRandomEngine();

	GridCell* GetCell(b2Vec2 pos);

	GridVertex* GetVertex(b2Vec2 pos);
	GridVertex* GetRandomVertex(const b2Vec2& position, float x);
	Path GetPath(GridVertex* begin, GridVertex* end);
};

class RavenScene : public SGE::Scene
{
protected:
	World world;
	SGE::Game* game = nullptr;
	std::string path;
	RavenGameState* gs = nullptr;

	static bool init();
public:
	SGE::Scene* endScene = nullptr;

	RavenScene(SGE::Game* game, const char* path);

	virtual void loadScene() override;
	virtual void unloadScene() override;

	virtual ~RavenScene();

	virtual void finalize() override;

	virtual void onDraw() override;
};

#endif
