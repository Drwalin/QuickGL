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

#include <algorithm>
#include <memory>
#include <vector>
#include <map>

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"

#include "../../include/quickgl/util/AllocatorVBO.hpp"

namespace qgl {
	AllocatorVBO::AllocatorVBO(uint32_t vertexSize,
			bool isElementOtherwiseVertexBuffer) :
		Allocator(new gl::VBO(vertexSize, isElementOtherwiseVertexBuffer ?
					gl::ELEMENT_ARRAY_BUFFER :
					gl::ARRAY_BUFFER,
					gl::STATIC_DRAW),
				Resize,
				Destroy) {
	}
	
	AllocatorVBO::~AllocatorVBO() {
	}
	
	void AllocatorVBO::Resize(void* obj, uint32_t size) {
		((gl::VBO*)obj)->Resize(size);
	}
	
	void AllocatorVBO::Destroy(void* obj) {
		delete (gl::VBO*)obj;
	}
}

