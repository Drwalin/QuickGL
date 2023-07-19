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

#include <chrono>
#include <set>
#include <thread>

#include "../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

#include "../include/quickgl/cameras/Camera.hpp"
#include "../include/quickgl/pipelines/Pipeline.hpp"
#include "../include/quickgl/Gui.hpp"
#include "../include/quickgl/util/DeltaVboManager.hpp"
#include "../include/quickgl/util/MoveVboUpdater.hpp"
#include "../include/quickgl/GlobalEntityManager.hpp"
#include "../include/quickgl/IndirectDrawBufferGenerator.hpp"

#include "../include/quickgl/Engine.hpp"

namespace qgl {
	Engine::Engine() {
		initialized = false;
		profiling = false;
	}
	
	Engine::~Engine() {
		Destroy();
	}
	
	void Engine::InitGL(std::string windowTitle) {
	GL_CHECK_PUSH_ERROR;
		gl::openGL.Init(windowTitle.c_str(), 800, 600, true, false,
				std::thread::hardware_concurrency()<=4,
				4, 2);
	GL_CHECK_PUSH_ERROR;
		gl::openGL.InitGraphic();
	GL_CHECK_PUSH_ERROR;
		inputManager.Init();
	GL_CHECK_PUSH_ERROR;
		Gui::InitIMGUI();
		initialized = true;
		
		deltaVboManager = std::make_shared<DeltaVboManager>(1024*1024*16, 16);
		deltaVboManager->Init();
		moveVboManager = std::make_shared<MoveVboManager>(shared_from_this());
		globalEntityManager = std::make_shared<GlobalEntityManager>(shared_from_this());
		
		indirectDrawBufferGenerator
			= std::make_shared<IndirectDrawBufferGenerator>(shared_from_this());
		indirectDrawBufferGenerator->Init();
	}
	
	void Engine::Destroy() {
		if(initialized) {
			gl::Finish();
			
			renderStageComposer.Destroy();

			for(auto& p : pipelines) {
				p = nullptr;
			}
			pipelines.clear();
			
			indirectDrawBufferGenerator->Destroy();
			indirectDrawBufferGenerator = nullptr;

			mainCamera = nullptr;
			
			globalEntityManager = nullptr;

			Gui::DeinitIMGUI();
			gl::openGL.Destroy();
			glfwTerminate();
			initialized = false;
		}
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
		pipeline->SetPipelineId(pipelines.size());
		pipeline->Init();
		renderStageComposer.AddPipeline(pipeline);
		return id;
	}
	
	std::shared_ptr<Pipeline> Engine::GetPipeline(uint32_t id) {
		if(id == 0 || id > pipelines.size())
			return nullptr;
		return pipelines[id-1];
	}
	
	void Engine::BeginNewFrame() {
		gl::openGL.InitFrame();
		inputManager.NewFrame();
		Gui::BeginNewFrame();
	}
	
	void Engine::Render() {
		for(auto c : cameras) {
			c->PrepareDataForNewFrame();
		}
		
		renderStageComposer.ResetExecution();
		while(renderStageComposer.HasAnyStagesLeft()) {
			if(renderStageComposer.ContinueStages() == false) {
				std::this_thread::sleep_for(std::chrono::microseconds(100));
			}
			// do some CPU work between render stages here
		}
	}
	
	void Engine::SwapBuffers() {
		Gui::EndFrame();
		gl::Flush();
		gl::openGL.SwapBuffer();
	}
	
	void Engine::AddCamera(std::shared_ptr<Camera> camera) {
		renderStageComposer.AddCamera(camera);
		cameras.insert(camera);
	}
	
	void Engine::RemoveCamera(std::shared_ptr<Camera> camera) {
		renderStageComposer.RemoveCamera(camera);
		cameras.erase(camera);
	}
	
	void Engine::SetMainCamera(std::shared_ptr<Camera> camera) {
		renderStageComposer.RenderAsLast(camera);
		mainCamera = camera;
		if(cameras.count(camera) == 0)
			cameras.insert(camera);
	}
	
	void Engine::PrintErrors() {
		// TODO: is it optional?
		gl::openGL.PrintErrors();
		gl::openGL.ClearErrors();
	}
	
	bool Engine::GetProfiling() const {
		return profiling;
	}
	
	void Engine::EnableProfiling(bool value) {
		profiling = value;
		renderStageComposer.SetGlFinishInEveryStageToProfile(profiling);
	}
	
	uint32_t Engine::GetEntitiesCount() const {
		uint32_t count = 0;
		for(auto p : pipelines) {
			count += p->GetEntitiesCount();
		}
		return count;
	}
	
	std::shared_ptr<DeltaVboManager> Engine::GetDeltaVboManager() {
		return deltaVboManager;
	}
	
	std::shared_ptr<MoveVboManager> Engine::GetMoveVboManager() {
		return moveVboManager;
	}
	
	std::shared_ptr<GlobalEntityManager> Engine::GetGlobalEntityManager() {
		return globalEntityManager;
	}
	
	std::shared_ptr<IndirectDrawBufferGenerator> Engine::GetIndirectDrawBufferGenerator() {
		return indirectDrawBufferGenerator;
	}
}

