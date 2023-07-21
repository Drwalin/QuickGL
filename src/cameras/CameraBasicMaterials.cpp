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

#include "../../include/quickgl/cameras/CameraBasicMaterials.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace qgl {
	CameraBasicMaterials::CameraBasicMaterials(uint32_t width, uint32_t height) :
			CameraBasic(width, height) {
		colorTexture = std::make_shared<gl::Texture>();
		colorTexture->UpdateTextureData(
				nullptr,
				width,
				height,
				false,
				gl::TEXTURE_2D,
				(gl::TextureSizedInternalFormat)gl::RGBA,
				gl::RGBA,
				gl::UNSIGNED_BYTE);
		colorTexture->SetDefaultParamPixelartClampBorderNoMipmap();
		fbo->AttachColor(colorTexture.get(), 0, 0);
	}
	
	CameraBasicMaterials::~CameraBasicMaterials() {
		fbo->DetachColor(0);
		colorTexture->Destroy();
		colorTexture = nullptr;
	}
	
	void CameraBasicMaterials::PrepareDataForNewFrame() {
		CameraBasic::PrepareDataForNewFrame();
	}
	
	std::shared_ptr<gl::Texture> CameraBasicMaterials::GetMainColorTexture() {
		return colorTexture;
	}
	
	void CameraBasicMaterials::SetRenderTargetDimensions(uint32_t width,
			uint32_t height) {
		CameraBasic::SetRenderTargetDimensions(width, height);
		colorTexture->UpdateTextureData(
				nullptr,
				width,
				height,
				false,
				gl::TEXTURE_2D,
				(gl::TextureSizedInternalFormat)gl::RGBA,
				gl::RGBA,
				gl::UNSIGNED_BYTE);
	}
}

