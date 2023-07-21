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

#ifndef QUICKGL_CAMERA_BASIC_MATERIALS_HPP
#define QUICKGL_CAMERA_BASIC_MATERIALS_HPP

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>

#include "CameraBasic.hpp"

namespace qgl {
	class Engine;
	
	class CameraBasicMaterials : public CameraBasic {
	public:
		
		CameraBasicMaterials(uint32_t width, uint32_t height);
		virtual ~CameraBasicMaterials();
		
		virtual void PrepareDataForNewFrame() override;
		
		virtual std::shared_ptr<gl::Texture> GetMainColorTexture() override;
		
		virtual void SetRenderTargetDimensions(uint32_t width, uint32_t height) override;
		
	protected:
		
		std::shared_ptr<gl::Texture> colorTexture;
	};
}

#endif

