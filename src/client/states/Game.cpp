﻿#include "Game.hpp"
#include <iostream>

Game::Game(GameManager &gm)
	: GameInterface(gm), menu_focus(false),
	_player(),
	btnResume(std::string("Back to game"), glm::ivec2(0, 24), glm::ivec2(284, 44)),
	btnOption(std::string("Option"), glm::ivec2(0, 24 + 44 + 10), glm::ivec2(284, 44)),
	btnMain(std::string("save and to menu"), glm::ivec2(0, 24 + (44 + 10) * 2 ), glm::ivec2(284, 44)), shader(ResourceManager::get().getShader("voxel"))
{
	texture = ResourceManager::get().getTexture("awesomeface.png", true);
	_gm->window->displayCursor(false);
	// test	
	std::vector<Vertex> vert;
	vert.push_back(Vertex { glm::vec3(0,0,0), glm::vec3(1,0,1) });
	vert.push_back(Vertex { glm::vec3(0,1,0), glm::vec3(1,0,1) });
	vert.push_back(Vertex { glm::vec3(1,0,0), glm::vec3(1,0,1) });

	vert.push_back(Vertex { glm::vec3(1,1,0), glm::vec3(1,0,1) });
	vert.push_back(Vertex { glm::vec3(1,0,0), glm::vec3(1,0,1) });
	vert.push_back(Vertex { glm::vec3(0,1,0), glm::vec3(1,0,1) });

	_cube = std::make_unique<Mesh>(shader);
	_cube->setVertice(vert);
	_cube->load();
	
	_camera = std::make_unique<Camera>();

	_server.run();
	_client.run();
	std::cout << "Connection Serveur" << std::endl;
}

Game::~Game()
{
	_server.stop();
	_client.stop();
}

void Game::render(int fps)
{
	// 3D
	glEnable(GL_DEPTH_TEST); // activer la profondeur
	if (debug)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);

	shader.use().set("view", _camera->lookAt(_player.getPos() + glm::vec3(0,1,0)));
	glm::mat4 model(1);
	model = glm::translate(model, glm::vec3(3));
	_cube->draw(model);
	_world.render();
	_player.render(_camera.get());

	// interface
	std::string fpsString = std::to_string(fps) + " FPS";
	ResourceManager::get().text.RenderText(fpsString, 10, 10, 1, glm::vec3(0.7, 0.1, 0.1));
	glm::vec3 pos = _player.getPos();
	ResourceManager::get().text.RenderText("pos: " + std::to_string((int)pos.x)+":" + std::to_string((int)pos.y)+":" + std::to_string((int)pos.z), 10, 30, 1, glm::vec3(0.7, 0.1, 0.1));
	ResourceManager::get().text.RenderText("Petit pierre", 10, 124, 1, glm::vec3(0.1, 0.1, 0.1));
	ResourceManager::get().sprit.DrawSprite(texture, glm::vec2(_gm->window->getWidth() / 2 - 13, _gm->window->getHeight() / 2 - 13), glm::vec2(27, 27), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));

	if (menu_focus) {
		btnResume.render();
		btnOption.render();
		btnMain.render();
	}
}

void Game::update(float deltatime)
{
	_player.updateInput(deltatime, _camera.get(), &_world);
	
	// verification 
	if (!Input::isFocus() || Input::getKey(GLFW_KEY_ESCAPE) == 2) {
		_gm->window->displayCursor(true);
		menu_focus = true;
	}

	// Option pour afficher en vecteur
	if (Input::getKey(GLFW_KEY_F3) == 1) {
		if (!debug)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		debug = !debug;
	}

	// game update
	if (!menu_focus) {
		
		if (Input::getMouse(GLFW_MOUSE_BUTTON_LEFT) == 1) {
			glm::vec4 block = _world.TraceRay(_player.getPos() + glm::vec3(0, 1.5, 0), _camera.get()->get_front(), 100);
			std::cout << "colition vel: " << (int)block.x << ":" << (int)block.y << ":" << (int)block.z << " w:" << (int)block.w << std::endl;
			if (block.w != -1)
				_world.setBlock(block.x, block.y, block.z, 0);
		}

		if (Input::getMouse(GLFW_MOUSE_BUTTON_RIGHT)) {

		}

		_camera->update(Input::getCursorRel());

		return;
	}

	// menu
	if (btnResume.onClick() && Input::getMouse(GLFW_MOUSE_BUTTON_1) == 1) {
		_gm->window->displayCursor(false);
		menu_focus = false;
	}
	if (btnMain.onClick() && Input::getMouse(GLFW_MOUSE_BUTTON_1) == 1)
		_gm->setState<StateMain>(*_gm);

}

void Game::resize(int w, int h)
{
	shader.use().set("projection", glm::perspective(70.0f, (GLfloat)w / (GLfloat)h, 0.1f, 100000.0f));
	btnResume.resize(w, h);
	btnOption.resize(w, h);
	btnMain.resize(w, h);
}

bool Game::end()
{
	return (_gm->window->closeButton()) ? true : false;
}