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

#ifndef QUICKGL_ALLOCATOR_VBO_HPP
#define QUICKGL_ALLOCATOR_VBO_HPP

#include <memory>
#include <vector>
#include <map>

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"

namespace qgl {
	class AllocatorVBO {
	public:
		
		AllocatorVBO(uint32_t elementSize, bool isElementOtherwiseVertexBuffer);
		~AllocatorVBO();
		
		uint32_t Allocate(uint32_t sizeElements);
		void Free(uint32_t posElements, uint32_t sizeElements);
		
		inline gl::VBO& VBO() { return vbo; }
		
	private:
		
		std::map<uint32_t, uint32_t> freeRanges;
		
		gl::VBO vbo;
	};
}

#endif

