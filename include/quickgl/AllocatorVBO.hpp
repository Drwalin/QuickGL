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

#include <cinttypes>
#include <vector>
#include <map>

namespace gl {
	class VBO;
}

namespace qgl {
	class AllocatorVBO {
	public:
		
		void Init(void* bufferObject,
				void(*resize)(void*, uint32_t newSize),
				void(*destructor)(void*));
		
		AllocatorVBO(uint32_t vertexSize, bool isElementOtherwiseVertexBuffer);
		AllocatorVBO(const char* debug);
		AllocatorVBO(void* bufferObject,
				void(*resize)(void*, uint32_t newSize),
				void(*destructor)(void*));
		~AllocatorVBO();
		
		uint32_t Allocate(uint32_t count);
		void Free(uint32_t ptr, uint32_t count);
		void ReserveAdditional(uint32_t additionalElements);
		
	private:
		
		std::map<uint32_t, uint32_t> freeRanges; // { offset, size }
		uint32_t allocated;
		void* bufferObject;
		void(*resize)(void*, uint32_t newSize);
		void(*destructor)(void*);
	};
}

#endif

