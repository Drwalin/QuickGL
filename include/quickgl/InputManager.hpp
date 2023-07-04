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

#ifndef QUICKGL_INPUT_MANAGER_HPP
#define QUICKGL_INPUT_MANAGER_HPP

#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "util/Log.hpp"

class GLFWwindow;

namespace qgl {
	class InputManager {
	public:
		
		InputManager() = default;
		~InputManager() = default;
		
		void Init();
		
		void NewFrame();
		
		std::string GetClipboardContent() const;
		void SetClipboardContent(std::string value);
		
		void LockMouse();
		void UnlockMouse();
		
		glm::vec3 GetMouseDelta() const;
		glm::vec3 GetMousePos() const;
		
		bool IsKeyDown(const int id) const;
		bool IsKeyUp(const int id) const;
		bool WasKeyPressed(const int id) const;
		bool WasKeyReleased(const int id) const;
		
		inline float GetTime() { return time; }
		inline float GetDeltaTime() { return deltaTime; }
		
	private:
		
		float time, deltaTime;
	};
}

#endif

