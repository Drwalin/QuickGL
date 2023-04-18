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

#include "../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

#include "../include/quickgl/cameras/Camera.hpp"
#include "../include/quickgl/pipelines/Pipeline.hpp"
#include "../include/quickgl/Gui.hpp"

#include "../include/quickgl/Engine.hpp"

namespace qgl {
	Engine::Engine() {
	}
	
	Engine::~Engine() {
	}
	
	
	void Engine::InitGL(std::string windowTitle) {
	GL_CHECK_PUSH_ERROR;
		gl::openGL.Init(windowTitle.c_str(), 800, 600, true, false, true, 4, 5);
	GL_CHECK_PUSH_ERROR;
		gl::openGL.InitGraphic();
	GL_CHECK_PUSH_ERROR;
		inputManager.Init();
	GL_CHECK_PUSH_ERROR;
		Gui::InitIMGUI();
	}
	
	void Engine::Destroy() {
		for(auto& p : pipelines) {
			p = NULL;
		}
		pipelines.clear();
		
		mainCamera = NULL;
		
		Gui::DeinitIMGUI();
		gl::openGL.Destroy();
		glfwTerminate();
	}
	
	
	void Engine::SetFullscreen(bool fullscreen) {
		gl::openGL.SetFullscreen(fullscreen);
	}
	
	
	void Engine::SetWindowTitle(std::string title) {
		glfwSetWindowTitle(gl::openGL.window, title.c_str());
	}
	
	
	bool Engine::IsQuitRequested() {
		return glfwWindowShouldClose(gl::openGL.window);
	}
	
	
	int32_t Engine::AddPipeline(std::shared_ptr<Pipeline> pipeline) {
		int32_t id = pipelines.size();
		pipelines.emplace_back(pipeline);
		pipeline->Initialize();
		renderStageComposer.AddPipelineStages(pipeline);
		return id;
	}
	
	std::shared_ptr<Pipeline> Engine::GetPipeline(int32_t id) {
		if(id < 0 || id >= pipelines.size())
			return nullptr;
		return pipelines[id];
	}
	
	
	void Engine::BeginNewFrame() {
		gl::openGL.InitFrame();
		inputManager.NewFrame();
		Gui::BeginNewFrame();
	}
	
	void Engine::Render() {
		mainCamera->PrepareDataForNewFrame();
		
		renderStageComposer.RestartStages();
		bool end = false;
		do {
			end = renderStageComposer.NextStage(mainCamera) > 0 ? false : true;
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		} while(!end);
	}
	
	void Engine::SwapBuffers() {
		Gui::EndFrame();
		glFlush();
		gl::openGL.SwapBuffer();
	}
	
	void Engine::SetMainCamera(std::shared_ptr<Camera> camera) {
		mainCamera = camera;
	}
	
	void Engine::PrintErrors() {
		// TODO: is it optional?
		gl::openGL.PrintErrors();
		gl::openGL.ClearErrors();
	}
}

