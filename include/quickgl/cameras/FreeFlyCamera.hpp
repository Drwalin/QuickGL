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

#ifndef QUICKGL_FREE_FLY_CAMERA_HPP
#define QUICKGL_FREE_FLY_CAMERA_HPP

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>

#include "Camera.hpp"

namespace qgl {
	class Engine;
	
	class FreeFlyCamera : public Camera {
	public:
		
		FreeFlyCamera();
		virtual ~FreeFlyCamera();
		
		virtual void PrepareDataForNewFrame() override;
		
		virtual void SetRenderTargetDimensions(uint32_t width, uint32_t height) override;
		virtual void GetRenderTargetDimensions(uint32_t& width, uint32_t& height) override;
		virtual void SetFov(float fov) override;
		virtual float GetFov() override;
		virtual glm::mat4 GetPerspectiveMatrix() override;
		
		
		virtual glm::mat4 GetViewMatrix() override;
		
		virtual glm::vec3 GetPosition() override;
		virtual void SetPosition(glm::vec3 position) override;
		
		virtual glm::vec3 GetFront() override;
		virtual glm::vec3 GetRight() override;
		virtual glm::vec3 GetUp() override;
		
		virtual void GetClippingPlanes(glm::vec3 normals[5], float offsets[5]) override;
		
		void Rotate(glm::vec3 euler);
		void SetRotation(glm::vec3 euler);
		virtual void Rotate(glm::quat rotation) override;
		virtual void SetRotation(glm::quat rotation) override;
		virtual glm::mat3 GetRotationMatrix() override;
		
		void ProcessDefaultInput(std::shared_ptr<Engine> engine);
		
	private:
		
		float fov;
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
	};
}

#endif


