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

#ifndef QUICKGL_ENGINE_HPP
#define QUICKGL_ENGINE_HPP

#include <memory>
#include <vector>

#include "InputManager.hpp"
#include "util/RenderStageComposer.hpp"
#include "util/Log.hpp"

namespace qgl {
	class Pipeline;
	class Camera;
	class DeltaVboManager;
	class MoveVboManager;
	class GlobalEntityManager;
	class IndirectDrawBufferGenerator;
	class BlitCameraToScreen;
	
	class Engine : public std::enable_shared_from_this<Engine> {
	public:
		
		Engine();
		~Engine();
		
		void InitGL(std::string windowTitle);
		void Destroy();
		
		void SetWindowTitle(std::string title);
		
		void SetFullscreen(bool fullscreen);
		
		bool IsQuitRequested();
		
		int32_t AddPipeline(std::shared_ptr<Pipeline> pipeline);
		std::shared_ptr<Pipeline> GetPipeline(uint32_t id);
		
		void BeginNewFrame();
		void Render();
		void SwapBuffers();
		
		uint32_t GetEntitiesCount() const;
		
		void AddCamera(std::shared_ptr<Camera> camera);
		void RemoveCamera(std::shared_ptr<Camera> camera);
		void SetMainCamera(std::shared_ptr<Camera> camera);
		
		inline InputManager& GetInputManager() { return inputManager; }
		
		void PrintErrors();
		
		inline const std::vector<StageTiming> GetTimings() const { return renderStageComposer.GetTimings(); }
		inline double CountCpuTime() const { return renderStageComposer.GetTotalCpuTime(); }
		
		void EnableProfiling(bool value);
		bool GetProfiling() const;
		
		std::shared_ptr<DeltaVboManager> GetDeltaVboManager();
		std::shared_ptr<MoveVboManager> GetMoveVboManager();
		
		std::shared_ptr<GlobalEntityManager> GetGlobalEntityManager();
		std::shared_ptr<IndirectDrawBufferGenerator> GetIndirectDrawBufferGenerator();
		
		std::shared_ptr<BlitCameraToScreen> GetBlitter() { return blitTexture; }
		
	protected:
		
		bool profiling;
		
		bool initialized;
		
		InputManager inputManager;
		RenderStageComposer renderStageComposer;
		
		std::vector<std::shared_ptr<Pipeline>> pipelines;
		std::shared_ptr<Camera> mainCamera;
		std::set<std::shared_ptr<Camera>> cameras;
		
		std::shared_ptr<DeltaVboManager> deltaVboManager;
		std::shared_ptr<MoveVboManager> moveVboManager;
		
		std::shared_ptr<GlobalEntityManager> globalEntityManager;
		
		std::shared_ptr<IndirectDrawBufferGenerator> indirectDrawBufferGenerator;
		
		std::shared_ptr<BlitCameraToScreen> blitTexture;
		
		std::shared_ptr<Pipeline> pipelinePostProcessing;
	};
}

#endif

