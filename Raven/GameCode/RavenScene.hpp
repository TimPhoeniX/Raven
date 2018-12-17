#ifndef ZOMBIE_SCENE
#define ZOMBIE_SCENE

#include <Game/sge_game.hpp>
#include <Scene/sge_scene.hpp>
#include "MovingObject.hpp"
#include "World.hpp"

namespace SGE
{
	class RealSpriteBatch;
}

class RavenScene: public SGE::Scene
{
protected:
	World world;
	SGE::Game* game = nullptr;
	std::string path;
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
