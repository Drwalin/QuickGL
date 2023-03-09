/*
 *  This file is part of QuickGL.
 *  Copyright (C) 2023 Marek Zalewski aka Drwalin
 *
 *  QuickGL is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  QuickGL is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <set>

#include "../include/quickgl/Engine.hpp"
#include "../include/quickgl/cameras/Camera.hpp"
#include "../include/quickgl/pipelines/Pipeline.hpp"

#include "../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

namespace qgl {
	Engine::Engine() {
	}
	
	Engine::~Engine() {
	}
	
	
	void Engine::InitGL(std::string windowTitle) {
		gl::openGL.Init(windowTitle.c_str(), 800, 600, true, false);
		gl::openGL.InitGraphic();
		inputManager.Init();
	}
	
	void Engine::Destroy() {
		for(auto& p : pipelines) {
			p = NULL;
		}
		mainCamera = NULL;
		
		gl::openGL.Destroy();
		glfwTerminate();
	}
	
	
	void Engine::SetWindowTItle(std::string title) {
		glfwSetWindowTitle(gl::openGL.window, title.c_str());
	}
	
	
	bool Engine::IsQuitRequested() {
		return glfwWindowShouldClose(gl::openGL.window);
	}
	
	void Engine::ProcessInput() {
		inputManager.NewFrame();
	}
	
	
	int32_t Engine::AddPipeline(std::shared_ptr<Pipeline> pipeline) {
		int32_t id = pipelines.size();
		pipelines.emplace_back(pipeline);
		return id;
	}
	
	std::shared_ptr<Pipeline> Engine::GetPipeline(int32_t id) {
		if(id < 0 || id >= pipelines.size())
			return nullptr;
		return pipelines[id];
	}
	
	
	void Engine::Render() {
		mainCamera->PrepareDataForNewFrame();
		
		std::set<std::shared_ptr<Pipeline>> pendingRenders(pipelines.begin(),
				pipelines.end()), nextStage;
		
		uint32_t drawStageId = 0;
		while(!pendingRenders.empty()) {
			nextStage.clear();
			for(auto pipeline : pendingRenders) {
				if(pipeline->DrawStage(mainCamera, drawStageId) > 0) {
					nextStage.insert(pipeline);
				}
			}
			std::swap(pendingRenders, nextStage);
			++drawStageId;
		}
		
	}
	
	void Engine::SetMainCamera(std::shared_ptr<Camera> camera) {
		mainCamera = camera;
	}
}

