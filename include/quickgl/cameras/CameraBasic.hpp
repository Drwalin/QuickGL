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

#ifndef QUICKGL_CAMERA_BASIC_HPP
#define QUICKGL_CAMERA_BASIC_HPP

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>

#include "../../../OpenGLWrapper/include/openglwrapper/FBO.hpp"

#include "Camera.hpp"

namespace qgl {
	class Engine;
	
	class CameraBasic : public Camera {
	public:
		
		CameraBasic(uint32_t width, uint32_t height);
		virtual ~CameraBasic();
		virtual void UseFbo() override;
		virtual void UnuseFbo() override;
		
		virtual void PrepareDataForNewFrame() override;
		
		virtual std::shared_ptr<gl::Texture> GetMainColorTexture() override;
		virtual std::shared_ptr<gl::Texture> GetDepthTexture() override;
		
		virtual void SetRenderTargetDimensions(uint32_t width, uint32_t height) override;
		virtual void GetRenderTargetDimensions(uint32_t& width, uint32_t& height) const override;
		virtual void SetFov(float fov) override;
		virtual float GetFov() const override;
		virtual float GetNear() const override;
		virtual float GetFar() const override;
		
		virtual glm::mat4 GetPerspectiveMatrix() const override;
		virtual glm::mat4 GetPerspectiveViewMatrix() const override;
		virtual glm::mat4 GetPreviousPerspectiveViewMatrix() const override;
		
		virtual void Clear(bool clearColor) override;
		
		virtual glm::mat4 GetViewMatrix() const override;
		
		virtual glm::vec3 GetPosition() const override;
		virtual void SetPosition(glm::vec3 position) override;
		
		virtual glm::vec3 GetFront() const override;
		virtual glm::vec3 GetRight() const override;
		virtual glm::vec3 GetUp() const override;
		
		virtual void GetClippingPlanes(glm::vec3 normals[5], float offsets[5]) const override;
		virtual void GetClippingPlanes(glm::vec4 normalsOffsets[5]) const override;
		
		virtual void Rotate(glm::vec3 euler) override;
		virtual void SetRotation(glm::vec3 euler) override;
		virtual void Rotate(glm::quat rotation) override;
		virtual void SetRotation(glm::quat rotation) override;
		virtual glm::mat3 GetRotationMatrix() const override;
		
	protected:
		
		std::shared_ptr<gl::FBO> fbo;
		std::shared_ptr<gl::Texture> depthTexture;
		
		float near, far;
		
		float fov, fovy;
		float aspectRatio;
		
		glm::mat4 view;
		glm::mat4 perspective;
		glm::mat4 transform;
		
		glm::quat rotation;
		
		glm::vec3 euler;
		
		glm::vec3 pos;
		
		glm::vec3 front;
		glm::vec3 up;
		glm::vec3 right;
		
		glm::vec4 clippingPlanes[5]; // {n.x, n.y, n.z, distance from origin}
		
		glm::mat4 perspectiveView;
		glm::mat4 previousPerspectiveView;
	};
}

#endif

