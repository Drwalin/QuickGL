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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace qgl {
	class Camera {
	public:
		
		Camera() = default;
		virtual ~Camera() = default;
		
		virtual void PrepareDataForNewFrame() = 0;
		
		virtual void SetRenderTargetDimensions(uint32_t width, uint32_t height) = 0;
		virtual void GetRenderTargetDimensions(uint32_t& width, uint32_t& height) = 0;
		virtual void SetFov(float fov) = 0;
		virtual float GetFov() = 0;
		virtual glm::mat4 GetPerspectiveMatrix() = 0;
		
		
		virtual glm::mat4 GetViewMatrix() = 0;
		
		virtual glm::vec3 GetPosition() = 0;
		virtual void SetPosition(glm::vec3 position) = 0;
		
		virtual glm::vec3 GetFront() = 0;
		virtual glm::vec3 GetRight() = 0;
		virtual glm::vec3 GetUp() = 0;
		
		virtual void GetClippingPlanes(glm::vec3 normals[5], float offsets[5]) = 0;
		
		virtual void Rotate(glm::quat rotation) = 0;
		virtual void SetRotation(glm::quat rotation) = 0;
		virtual glm::mat3 GetRotationMatrix() = 0;
	};
}

#endif


