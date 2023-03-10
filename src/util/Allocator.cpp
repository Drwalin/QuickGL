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

#include "../../include/quickgl/util/Allocator.hpp"

namespace qgl {
	Allocator::Allocator(void* bufferObject,
			void(*resize)(void*, uint32_t newSize),
			void(*destructor)(void*)) {
		Init(bufferObject, resize, destructor);
	}
	
	void Allocator::Init(void* bufferObject,
			void(*resize)(void*, uint32_t newSize),
			void(*destructor)(void*)) {
		this->allocated = 0;
		this->bufferObject = bufferObject;
		this->resize = resize;
		this->destructor = destructor;
	}
	
	Allocator::~Allocator() {
		destructor(bufferObject);
		bufferObject = NULL;
	}
	
	uint32_t Allocator::Allocate(uint32_t size) {
		if(freeRanges.size() == 0) {
			ReserveAdditional(
					std::max(std::max(size, allocated/2), 4096u));
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
			if(p.first + p.second < allocated) { // last free block is NOT at the end of allocated memory
				ReserveAdditional(
						std::max(std::max(size, allocated/2),
							4096u));
			} else { // last free block is at the end of allocated memory
				ReserveAdditional(
						std::max(std::max(size-p.second,
								allocated), 4096u));
			}
		}
		
		// after above code, assumes that last block has enaugh space
		auto it = freeRanges.rbegin();
		uint32_t offset = it->first;
		uint32_t r = it->second - size;
		freeRanges.erase(it->first);
		if(r > 0) {
			freeRanges[offset+size] = r;
		}
		return offset;
	}
	
	void Allocator::Free(uint32_t pos, uint32_t size) {
		freeRanges[pos] = size;
		if(freeRanges.size() == 0) {
		} else {
			freeRanges[pos] = size;
			auto it = freeRanges.find(pos);
			{
				auto next = it; ++next;
				if(next != freeRanges.end()) { // if freed block is not last
					if(pos+size == next->first) { // if freed block ends where next starts
						it->second += next->second;
						freeRanges.erase(next);
						it = freeRanges.find(pos);
					}
				}
			}
			if(it != freeRanges.begin()) { // if freed block is not first
				auto prev = it; --prev;
				if(prev->first + prev->second == pos) { // if previous block ends where freed starts
					prev->second += it->second;
					freeRanges.erase(pos);
				}
			}
		}
	}
	
	void Allocator::ReserveAdditional(uint32_t additionalElements) {
		uint32_t prevSize = allocated;
		uint32_t newSize = prevSize+additionalElements;
		if(resize) {
			resize(bufferObject, newSize);
		}
		Free(prevSize, additionalElements);
		allocated = newSize;
	}
}

