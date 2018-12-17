//
//  main.cpp
//  ZombieGame
//
//  Created by Hamashy on 13.02.2016.
//  Copyright © 2016 Hamashy. All rights reserved.
//

#include <iostream>
#include <boost/filesystem.hpp>

#include <Game/sge_game.hpp>
#include <Game/Director/sge_director.hpp>
#include <Scene/sge_scene.hpp>

#include "GameCode/RavenScene.hpp"
#include "GameCode/IntroScene.hpp"

namespace fs = boost::filesystem;

int main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    std::cout.setf(std::ios::boolalpha);
    std::cout.sync_with_stdio(true);

	const fs::path app_path = fs::system_complete(argv[0]).remove_filename();
	const fs::path shader_path = app_path / fs::path("shaders");

	SGE::Director* director = SGE::Director::getDirector(1024, 768);
	SGE::Game* game = SGE::Game::getGame();
	game->bindDirector(director);
	game->init(60, shader_path.string());
	game->setGamePath({app_path.string() + '/'});
	game->setShadersPath({shader_path.string() + '/'});

	RavenScene* S1 = new RavenScene(game, "Levels/level1.txt");
	SGE::Scene* S0 = new IntroScene(S1, "Resources/Textures/zombie-game.png");
	SGE::Scene* S2 = new EndScene(S1, "Resources/Textures/end-game.png", "Resources/Textures/lost-game.png");

	S1->endScene = S2;

	director->addScene(S0);
	director->addScene(S1);
	director->addScene(S2);

	director->setNextScene(S0);

	game->run();

	director->deleteScene(S0);
	director->deleteScene(S1);
	director->deleteScene(S2);

	game->finalize();
	director->finalize();

    return 0;
}
