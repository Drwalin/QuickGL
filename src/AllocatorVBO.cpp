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

#include "../include/quickgl/AllocatorVBO.hpp"

namespace qgl {
	AllocatorVBO::AllocatorVBO(uint32_t elementSize,
			bool isElementOtherwiseVertexBuffer) :
		vbo(elementSize, isElementOtherwiseVertexBuffer ?
				gl::ELEMENT_ARRAY_BUFFER :
				gl::ARRAY_BUFFER,
				gl::STATIC_DRAW) {
	}
	
	AllocatorVBO::~AllocatorVBO() {
	}
	
	uint32_t AllocatorVBO::Allocate(uint32_t size) {
		if(freeRanges.size() == 0) {
			ReserveAdditional(
					std::max(std::max(size, vbo.GetVertexCount()/2), 4096u));
		} else {
			for(auto it : freeRanges) {
				if(it.second >= size) {
					uint32_t offset = it.first;
					uint32_t elements = it.second;
					
					freeRanges.erase(offset);
					if(elements > size) {
						freeRanges[offset+size] = elements-size;
					}
					
					return offset;
				}
			}
		
			auto p = *freeRanges.rbegin();
			if(p.first + p.second < vbo.GetVertexCount()) {
				ReserveAdditional(
						std::max(std::max(size, vbo.GetVertexCount()/2),
							4096u));
			} else {
				ReserveAdditional(
						std::max(std::max(size-p.second,
								vbo.GetVertexCount()/2), 4096u));
			}
		}
		auto it = freeRanges.rbegin();
		uint32_t offset = it->first;
		uint32_t r = it->second - size;
		freeRanges.erase(it->first);
		if(r > 0) {
			freeRanges[offset+size] = r;
		}
		return offset;
	}
	
	void AllocatorVBO::Free(uint32_t pos, uint32_t size) {
		auto it = freeRanges.upper_bound(pos);
	}
	
	void AllocatorVBO::ReserveAdditional(uint32_t elements) {
		
	}
}

