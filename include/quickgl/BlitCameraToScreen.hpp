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

#ifndef QUICKGL_BLIT_CAMERA_TO_SCREEN_HPP
#define QUICKGL_BLIT_CAMERA_TO_SCREEN_HPP

#include <memory>

namespace gl {
	class VBO;
	class Shader;
	class VAO;
	class Texture;
}

namespace qgl {
	class Camera;
	
	class BlitCameraToScreen final {
	public:
		
		BlitCameraToScreen();
		
		void Blit(std::shared_ptr<gl::Texture> texture,
				uint32_t width,
				uint32_t height);
		
	private:
		
		uint32_t textureLocation;
		
		std::shared_ptr<gl::Shader> shader;
		std::shared_ptr<gl::VBO> vbo;
		std::shared_ptr<gl::VAO> vao;
	};
}

#endif

