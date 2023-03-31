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

#include <glm/gtc/quaternion.hpp>

#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

#include "../../include/quickgl/Engine.hpp"
#include "../../include/quickgl/InputManager.hpp"
#include "../../include/quickgl/cameras/FreeFlyCamera.hpp"

namespace qgl {
	FreeFlyCamera::FreeFlyCamera() {
		fov = 90;
		aspectRatio = 1;
		pos = {0,0,0};
		front = {0,0,1};
		rotation = glm::quat(0, {0,1,0});
		euler = {0,0,0};
		near = 0.1;
		far = 10000;
	}
	
	FreeFlyCamera::~FreeFlyCamera() {
	}
	
	
	void FreeFlyCamera::PrepareDataForNewFrame() {
		glm::mat4 rot = glm::mat4_cast(rotation);
		front = rot * glm::vec4{0,0,1,0};
		up = rot * glm::vec4{0,1,0,0};
		right = rot * glm::vec4{-1,0,0,0};
		
		view = glm::lookAt(pos, pos+front, up);
		perspective = glm::perspective(fov*0.5f, aspectRatio, near, far);
		transform = glm::translate(rot, pos);
		
		{
			glm::mat4 invPV = glm::inverse(perspective*view);
			
			glm::vec4 _p[4] = {
				invPV * glm::vec4{-1,-1,1,1},
				invPV * glm::vec4{-1,1,1,1},
				invPV * glm::vec4{1,1,1,1},
				invPV * glm::vec4{1,-1,1,1},
			};
			
			glm::vec3 p[4];
			for(int i=0; i<4; ++i) {
				p[i] = _p[i];
			}
			
			glm::vec3 p0 = pos;
			
			for(int i=0; i<4; ++i) {
				glm::vec3 a = p[i%4];
				glm::vec3 b = p[(i+1)%4];
				glm::vec3 c = p[(i+2)%4];
				glm::vec3 n = glm::cross(a-p0, b-p0);
				if(glm::dot(n, c-p0) < 0)
					n = -n;
				float d = glm::dot(p0, n);
				clippingPlanes[i] = {n.x, n.y, n.z, d};
			}
			{
				glm::vec3 a = p[0];
				glm::vec3 b = p[1];
				glm::vec3 c = p[2];
				glm::vec3 n = glm::cross(a-c, b-c);
				if(glm::dot(n, p0-a) < 0)
					n = -n;
				float d = glm::dot(a, n);
				clippingPlanes[4] = {n.x, n.y, n.z, d};
			}
		}
	}
		
	void FreeFlyCamera::SetRenderTargetDimensions(uint32_t width,
			uint32_t height) {
		aspectRatio = ((float)width)/((float)height);
	}
	
	void FreeFlyCamera::GetRenderTargetDimensions(uint32_t& width,
			uint32_t& height) {
		throw "FreeFlyCamera::GetRenderTargetDimensions is not implemented.";
	}
	
	void FreeFlyCamera::SetFov(float fov) {
		this->fov = fov;
	}
	
	float FreeFlyCamera::GetFov() {
		return fov;
	}
	
	glm::mat4 FreeFlyCamera::GetPerspectiveMatrix() {
		return perspective;
	}
	
	
	glm::mat4 FreeFlyCamera::GetViewMatrix() {
		return view;
	}
	
	glm::vec3 FreeFlyCamera::GetPosition() {
		return pos;
	}
	
	void FreeFlyCamera::SetPosition(glm::vec3 position) {
		pos = position;
	}
	
	glm::vec3 FreeFlyCamera::GetFront() {
		return front;
	}
	
	glm::vec3 FreeFlyCamera::GetRight() {
		return right;
	}
	
	glm::vec3 FreeFlyCamera::GetUp() {
		return up;
	}
	
	
	void FreeFlyCamera::GetClippingPlanes(glm::vec3 normals[5],
			float offsets[5]) {
		for(int i=0; i<5; ++i) {
			normals[i] = clippingPlanes[i];
			offsets[i] = clippingPlanes[i][3];
		}
	}
	
	void FreeFlyCamera::GetClippingPlanes(glm::vec4 normalsOffsets[5]) {
		for(int i=0; i<5; ++i) {
			normalsOffsets[i] = clippingPlanes[i];
		}
	}
	
	void FreeFlyCamera::Rotate(glm::vec3 euler) {
		SetRotation(this->euler + euler);
	}
	
	void FreeFlyCamera::SetRotation(glm::vec3 euler) {
		if(euler.x < -M_PI*0.5f)
			euler.x = -M_PI*0.5f;
		if(euler.x > M_PI*0.5f)
			euler.x = M_PI*0.5f;
		this->euler = euler;
		SetRotation(
				glm::quat(euler + glm::vec3{0,0,M_PI})
// 				glm::rotate(
// 					glm::rotate(
// 						glm::rotate(
// 							glm::quat(0,0,0,1)
// 							,
// 							euler.y, {0,1,0})
// 						,
// 						euler.x, {1,0,0})
// 					,
// 					euler.z, {0,0,1})
				);
	}
	
	void FreeFlyCamera::Rotate(glm::quat rotation) {
		SetRotation(this->rotation*rotation);
	}
	
	void FreeFlyCamera::SetRotation(glm::quat rotation) {
		this->rotation = rotation;
	}
	
	glm::mat3 FreeFlyCamera::GetRotationMatrix() {
		return glm::mat3_cast(rotation);
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
			dpx += glm::vec3{0,1,0};
		if(input.IsKeyDown(GLFW_KEY_Q))
			dpx += glm::vec3{0,-1,0};
		
		if(input.IsKeyDown(GLFW_KEY_LEFT))
			dr.y -= 1.0f;
		if(input.IsKeyDown(GLFW_KEY_RIGHT))
			dr.y += 1.0f;
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
		
		dr.x += input.GetMouseDelta().y * 0.01;
		dr.y -= input.GetMouseDelta().x * 0.01;
		
// 		PrepareDataForNewFrame();
		
		Rotate(dr);
		SetPosition(GetPosition() + (glm::mat3(right, up, front))*dp - dpx);
	}
}

