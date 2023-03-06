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

#ifndef QUICKGL_MESH_CATEGORY_RENDERER_HPP
#define QUICKGL_MESH_CATEGORY_RENDERER_HPP

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "../../OpenGLWrapper/include/openglwrapper/FBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VAO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Shader.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Texture.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

#include "util/AllocatorVBO.hpp"
#include "MeshManager.hpp"

namespace qgl {
	
	class MeshCategoryRenderer {
	public:
		
		MeshCategoryRenderer();
		~MeshCategoryRenderer();
		
	private:
		
		MeshManager meshManager;
		
		gl::VBO instanceData;
		gl::VBO indirectDrawBuffer;
	};
}

#endif

