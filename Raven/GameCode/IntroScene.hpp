#ifndef INTRO_SCENE
#define INTRO_SCENE

#include <Scene/sge_scene.hpp>

class IntroScene: public SGE::Scene
{
protected:
	std::string path;
	Scene* next;
public:
	IntroScene(Scene* next, const char* path);

	virtual void loadScene() override;

	virtual ~IntroScene();

	virtual void finalize() override;

	virtual void onDraw() override;
};

class EndScene: public IntroScene
{
	size_t winBatch = 0;
	size_t loseBatch = 0;
	std::string path2;
public:
	bool won = false;
	EndScene(Scene* next, const char* path, const char* path2);
	void onDraw() override;
	virtual void loadScene() override;
};

#endif
