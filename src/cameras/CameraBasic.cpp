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

#include "../../include/quickgl/cameras/CameraBasic.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace qgl {
	CameraBasic::CameraBasic(uint32_t width, uint32_t height) {
		SetFov(75.0f);
		aspectRatio = 1;
		pos = {0,0,0};
		front = {0,0,1};
		rotation = glm::angleAxis(0.0f, glm::vec3{0,1,0});
		euler = {0,0,0};
		near = 0.1;
		far = 100000;
		
		depthTexture = std::make_shared<gl::Texture>();
		depthTexture->UpdateTextureData(
				nullptr,
				width,
				height,
				false,
				gl::TEXTURE_2D,
				(gl::TextureSizedInternalFormat) GL_DEPTH24_STENCIL8,
				(gl::TextureDataFormat) GL_DEPTH_STENCIL,
				(gl::DataType) GL_UNSIGNED_INT_24_8);
		
		fbo = std::make_shared<gl::FBO>();
		fbo->AttachTexture(depthTexture.get(), gl::ATTACHMENT_DEPTH_STENCIL, 0);
	}
	
	std::shared_ptr<gl::Texture> CameraBasic::GetMainColorTexture() {
		return depthTexture;
	}
	
	CameraBasic::~CameraBasic() {
		fbo->DetachDepth();
		depthTexture->Destroy();
		depthTexture = nullptr;
		fbo->Destroy();
	}
	
	void CameraBasic::UseFbo() {
		fbo->Bind();
		fbo->Viewport(0, 0, depthTexture->GetWidth(), depthTexture->GetHeight());
	}
	
	void CameraBasic::UnuseFbo() {
		fbo->Unbind();
	}
	
	
	void CameraBasic::PrepareDataForNewFrame() {
		glm::mat4 rot = glm::mat4_cast(rotation);
		front = rot * glm::vec4{0,0,-1,0};
		up = rot * glm::vec4{0,1,0,0};
		right = rot * glm::vec4{1,0,0,0};
		
		perspective = glm::perspective(fovy, aspectRatio, near, far);
		
		{
			glm::vec3 pp;
			pp.z = far;
			pp.y = pp.z * tan(fovy/2.0f);
			pp.x = pp.y * aspectRatio;
			glm::vec3 p[4] = {
				{1,1,-1},
				{-1,1,-1},
				{-1,-1,-1},
				{1,-1,-1},
			};
			for(int i=0; i<4; ++i) {
				p[i] = p[i]*pp;
			}
			
			constexpr glm::vec3 p0 = {0,0,0};
			
			for(int i=0; i<4; ++i) {
				glm::vec3 a = p[i%4];
				glm::vec3 b = p[(i+1)%4];
				glm::vec3 c = p[(i+2)%4];
				glm::vec3 n = glm::normalize(glm::cross(a-p0, b-p0));
				if(glm::dot(n, c-p0) < 0)
					n = -n;
				float d = glm::dot(p0, n);
				clippingPlanes[i] = {n.x, n.y, n.z, d};
			}
			{
				glm::vec3 a = p[0];
				glm::vec3 b = p[1];
				glm::vec3 c = p[2];
				glm::vec3 n = glm::normalize(glm::cross(a-c, b-c));
				if(glm::dot(n, p0-a) < 0)
					n = -n;
				float d = glm::dot(a, n);
				clippingPlanes[4] = {n.x, n.y, n.z, d};
			}
		}
		
		transform = glm::translate(rot, pos);
		view = glm::inverse(rot) * glm::translate(glm::mat4(1), -pos);
	}
		
	void CameraBasic::SetRenderTargetDimensions(uint32_t width,
			uint32_t height) {
		aspectRatio = ((float)width)/((float)height);
		fovy = aspectRatio < 1 ? fov : fov / aspectRatio;
		depthTexture->UpdateTextureData(
				nullptr,
				width,
				height,
				false,
				gl::TEXTURE_2D,
				(gl::TextureSizedInternalFormat)GL_DEPTH24_STENCIL8,
				(gl::TextureDataFormat)GL_DEPTH_STENCIL,
				(gl::DataType)GL_UNSIGNED_INT_24_8);
	}
	
	void CameraBasic::GetRenderTargetDimensions(uint32_t& width,
			uint32_t& height) {
		width = depthTexture->GetWidth();
		height = depthTexture->GetHeight();
	}
	
	void CameraBasic::SetFov(float fov) {
		fov = glm::radians(fov);
		this->fov = fov;
		fovy = aspectRatio < 1 ? fov : fov / aspectRatio;
	}
	
	float CameraBasic::GetFov() {
		return fov;
	}
	
	glm::mat4 CameraBasic::GetPerspectiveMatrix() {
		return perspective;
	}
	
	void CameraBasic::Clear(bool clearColor) {
		fbo->Bind();
		fbo->Viewport(0, 0, depthTexture->GetWidth(), depthTexture->GetHeight());
		fbo->SetClearColor({0.2, 0.3, 1, 1});
		fbo->Clear(clearColor, true);
	}
	
	glm::mat4 CameraBasic::GetViewMatrix() {
		return view;
	}
	
	glm::vec3 CameraBasic::GetPosition() {
		return pos;
	}
	
	void CameraBasic::SetPosition(glm::vec3 position) {
		pos = position;
	}
	
	glm::vec3 CameraBasic::GetFront() {
		return front;
	}
	
	glm::vec3 CameraBasic::GetRight() {
		return right;
	}
	
	glm::vec3 CameraBasic::GetUp() {
		return up;
	}
	
	
	void CameraBasic::GetClippingPlanes(glm::vec3 normals[5],
			float offsets[5]) {
		for(int i=0; i<5; ++i) {
			normals[i] = clippingPlanes[i];
			offsets[i] = clippingPlanes[i][3];
		}
	}
	
	void CameraBasic::GetClippingPlanes(glm::vec4 normalsOffsets[5]) {
		for(int i=0; i<5; ++i) {
			normalsOffsets[i] = clippingPlanes[i];
		}
	}
	
	void CameraBasic::Rotate(glm::vec3 euler) {
		SetRotation(this->euler + euler);
	}
	
	void CameraBasic::SetRotation(glm::vec3 euler) {
		if(euler.x < -M_PI*0.5f)
			euler.x = -M_PI*0.5f;
		if(euler.x > M_PI*0.5f)
			euler.x = M_PI*0.5f;
		this->euler = euler;
		SetRotation(glm::quat(euler));
	}
	
	void CameraBasic::Rotate(glm::quat rotation) {
		SetRotation(this->rotation*rotation);
	}
	
	void CameraBasic::SetRotation(glm::quat rotation) {
		this->rotation = rotation;
	}
	
	glm::mat3 CameraBasic::GetRotationMatrix() {
		return glm::mat3_cast(rotation);
	}
}

