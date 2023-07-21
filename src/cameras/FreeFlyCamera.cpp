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

#include <cmath>

#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>

#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

#include "../../include/quickgl/Engine.hpp"
#include "../../include/quickgl/InputManager.hpp"

#include "../../include/quickgl/cameras/FreeFlyCamera.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace qgl {
	FreeFlyCamera::FreeFlyCamera() :
		CameraBasicMaterials(gl::openGL.GetWidth(), gl::openGL.GetHeight()) {
	}
	
	FreeFlyCamera::~FreeFlyCamera() {
	}
	
	void FreeFlyCamera::PrepareDataForNewFrame() {
		if(depthTexture->GetWidth() != gl::openGL.GetWidth() ||
				depthTexture->GetHeight() != gl::openGL.GetHeight()) {
			SetRenderTargetDimensions(gl::openGL.GetWidth(),
					gl::openGL.GetHeight());
		}
		
		CameraBasicMaterials::PrepareDataForNewFrame();
	}
	
	
	void FreeFlyCamera::ProcessDefaultInput(std::shared_ptr<Engine> engine) {
		InputManager& input = engine->GetInputManager();
		PrepareDataForNewFrame();
		
		SetRenderTargetDimensions(gl::openGL.GetWidth(),
				gl::openGL.GetHeight());
		
		glm::vec3 dp(0,0,0), dr(0,0,0), dpx(0,0,0);
		const float dt = input.GetDeltaTime();
		
		if(input.IsKeyDown(GLFW_KEY_W))
			dp += glm::vec3{0,0,1};
		if(input.IsKeyDown(GLFW_KEY_S))
			dp += glm::vec3{0,0,-1};
		if(input.IsKeyDown(GLFW_KEY_A))
			dp += glm::vec3{-1,0,0};
		if(input.IsKeyDown(GLFW_KEY_D))
			dp += glm::vec3{1,0,0};
		if(input.IsKeyDown(GLFW_KEY_SPACE))
			dp += glm::vec3{0,1,0};
		if(input.IsKeyDown(GLFW_KEY_LEFT_CONTROL))
			dp += glm::vec3{0,-1,0};
		
		if(input.IsKeyDown(GLFW_KEY_E))
			dpx += glm::vec3{0,-1,0};
		if(input.IsKeyDown(GLFW_KEY_Q))
			dpx += glm::vec3{0,+1,0};
		
		if(input.IsKeyDown(GLFW_KEY_LEFT))
			dr.y += 1.0f;
		if(input.IsKeyDown(GLFW_KEY_RIGHT))
			dr.y -= 1.0f;
		if(input.IsKeyDown(GLFW_KEY_DOWN))
			dr.x -= 1.0f;
		if(input.IsKeyDown(GLFW_KEY_UP))
			dr.x += 1.0f;
		
		if(input.IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
			dp *= 5.0f;
			dr *= 5.0f;
			dpx *= 5.0f;
		}
		
		dr *= dt;
		dp *= dt * 5.0f;
		dpx *= dt * 5.0f;
		
		dr.x -= input.GetMouseDelta().y * 0.003;
		dr.y -= input.GetMouseDelta().x * 0.003;
		
		Rotate(dr);
		SetPosition(GetPosition() + (glm::mat3(right, up, front))*dp - dpx);
	}
}

