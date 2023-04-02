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

namespace qgl {
	class Pipeline;
	class Camera;
	class Gui;
	
	class Engine {
	public:
		
		Engine();
		~Engine();
		
		void InitGL(std::string windowTitle);
		void Destroy();
		
		void SetWindowTitle(std::string title);
		
		void SetFullscreen(bool fullscreen);
		
		bool IsQuitRequested();
		
		void BeginNewFrame();
		
		void SetGui(std::shared_ptr<Gui> gui);
		
		
		int32_t AddPipeline(std::shared_ptr<Pipeline> pipeline);
		std::shared_ptr<Pipeline> GetPipeline(int32_t id);
		
		void Render();
		
		void SetMainCamera(std::shared_ptr<Camera> camera);
		
		inline InputManager& GetInputManager() { return inputManager; }
		
		void PrintErrors();
		
	protected:
		
		InputManager inputManager;
		RenderStageComposer renderStageComposer;
		
		std::vector<std::shared_ptr<Pipeline>> pipelines;
		std::shared_ptr<Camera> mainCamera;
		
		std::shared_ptr<Gui> currentGui;
	};
}

#endif

