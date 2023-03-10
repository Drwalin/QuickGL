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

#ifndef QUICKGL_ALLOCATOR_HPP
#define QUICKGL_ALLOCATOR_HPP

#include <cinttypes>
#include <vector>
#include <map>

namespace qgl {
	class Allocator {
	public:
		
		void Init(void* bufferObject,
				void(*resize)(void*, uint32_t newSize),
				void(*destructor)(void*));
		
		Allocator(void* bufferObject,
				void(*resize)(void*, uint32_t newSize),
				void(*destructor)(void*));
		~Allocator();
		
		uint32_t Allocate(uint32_t count);
		void Free(uint32_t ptr, uint32_t count);
		void ReserveAdditional(uint32_t additionalElements);
		
	protected:
		
		std::map<uint32_t, uint32_t> freeRanges; // { offset, size }
		uint32_t allocated;
		void* bufferObject;
		void(*resize)(void*, uint32_t newSize);
		void(*destructor)(void*);
	};
}

#endif

