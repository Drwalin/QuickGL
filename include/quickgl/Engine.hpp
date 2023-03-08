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
#include <map>

#include "InputManager.hpp"

namespace qgl {
	class Pipeline;
	class Camera;
	
	class Engine {
	public:
		
		Engine();
		~Engine();
		
		void InitGL();
		void Destroy();
		
		bool IsQuitRequested();
		
		void ProcessInput();
		
		
		int AddPipeline(std::shared_ptr<Pipeline> pipeline);
		std::shared_ptr<Pipeline> GetPipeline(int id);
		
		void Render();
		
		void SetMainCamera(std::shared_ptr<Camera> camera);
		
		inline InputManager& GetInputManager() { return inputManager; }
		
	protected:
		
		InputManager inputManager;
		
		std::map<int, class Pipeline*> pipelines;
		std::shared_ptr<Camera> mainCamera;
	};
}

#endif

