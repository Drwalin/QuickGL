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

#include "../include/quickgl/InputManager.hpp"

#include "../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

namespace qgl {
	void InputManager::Init() {
		glfwPollEvents();
		time = glfwGetTime();
		double x, y;
		glfwGetCursorPos(gl::openGL.window, &x, &y);
		
		deltaTime = 0.016;
	}
	
	void InputManager::NewFrame() {
		gl::openGL.SwapInput();
		for(int i=0; i<32; ++i)
			glfwPollEvents();
		
		float t2 = glfwGetTime();
		deltaTime = t2 - time;
		time = t2;
	}
	
	
	std::string InputManager::GetClipboardContent() const {
		return glfwGetClipboardString(nullptr);
	}
	
	void InputManager::SetClipboardContent(std::string value) {
		glfwSetClipboardString(nullptr, value.c_str());
	}
	
	
	void InputManager::LockMouse() {
		glfwSetInputMode(gl::openGL.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	
	void InputManager::UnlockMouse() {
		glfwSetInputMode(gl::openGL.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	
	
	glm::vec3 InputManager::GetMouseDelta() const {
		return {gl::openGL.GetMouseDX(), gl::openGL.GetMouseDY(),
				gl::openGL.GetScrollDelta()};
	}
	
	glm::vec3 InputManager::GetMousePos() const {
		return {gl::openGL.GetMouseX(), gl::openGL.GetMouseY(),
				gl::openGL.GetScroll()};
	}
	
	bool InputManager::IsKeyDown(const int id) const {
		return gl::openGL.IsKeyDown(id);
	}
	
	bool InputManager::IsKeyUp(const int id) const {
		return gl::openGL.IsKeyUp(id);
	}
	
	bool InputManager::WasKeyPressed(const int id) const {
		return gl::openGL.WasKeyPressed(id);
	}

	bool InputManager::WasKeyReleased(const int id) const {
		return gl::openGL.WasKeyReleased(id);
	}

}

