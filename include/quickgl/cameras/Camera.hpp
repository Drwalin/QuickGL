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

#ifndef QUICKGL_CAMERA_HPP
#define QUICKGL_CAMERA_HPP

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../util/Log.hpp"

namespace gl {
	class Texture;
}

namespace qgl {
	class PostProcess;
	
	class Camera : public std::enable_shared_from_this<Camera> {
	public:
		
		Camera();
		virtual ~Camera();
		
		virtual void PrepareDataForNewFrame() = 0;
		
		virtual std::shared_ptr<gl::Texture> GetMainColorTexture() = 0;
		virtual std::shared_ptr<gl::Texture> GetDepthTexture() = 0;
		
		virtual void UseFbo() = 0;
		virtual void UnuseFbo() = 0;
		
		virtual void SetRenderTargetDimensions(uint32_t width, uint32_t height) = 0;
		virtual void GetRenderTargetDimensions(uint32_t& width, uint32_t& height) const = 0;
		virtual void SetFov(float fov) = 0;
		virtual float GetFov() const = 0;
		virtual float GetNear() const = 0;
		virtual float GetFar() const = 0;
		
		virtual glm::mat4 GetPerspectiveMatrix() const = 0;
		virtual glm::mat4 GetPerspectiveViewMatrix() const = 0;
		virtual glm::mat4 GetPreviousPerspectiveViewMatrix() const = 0;
		
		virtual void Clear(bool clearColor) = 0;
		
		virtual glm::mat4 GetViewMatrix() const = 0;
		
		virtual glm::vec3 GetPosition() const= 0;
		virtual void SetPosition(glm::vec3 position) = 0;
		
		virtual glm::vec3 GetFront() const = 0;
		virtual glm::vec3 GetRight() const = 0;
		virtual glm::vec3 GetUp() const = 0;
		
		virtual void GetClippingPlanes(glm::vec3 normals[5], float offsets[5]) const = 0;
		virtual void GetClippingPlanes(glm::vec4 normalsOffsets[5]) const = 0;
		
		virtual void Rotate(glm::vec3 euler) = 0;
		virtual void SetRotation(glm::vec3 euler) = 0;
		virtual void Rotate(glm::quat rotation) = 0;
		virtual void SetRotation(glm::quat rotation) = 0;
		virtual glm::mat3 GetRotationMatrix() const = 0;
		
		virtual void DoPostprocessing();
		void AddPostProcess(std::shared_ptr<PostProcess> postProcess);
		
	protected:
		
		std::vector<std::shared_ptr<PostProcess>> postProcesses;
		
	};
}

#endif

