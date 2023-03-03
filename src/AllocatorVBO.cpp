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

#include "../include/quickgl/AllocatorVBO.hpp"



#include <cstdio>
// #define DEBUG {printf(" %s : %i\n", __FILE__, __LINE__); fflush(stdout);}
#define DEBUG


namespace qgl {
	AllocatorVBO::AllocatorVBO(uint32_t vertexSize,
			bool isElementOtherwiseVertexBuffer) {
		Init(
				new gl::VBO(vertexSize, isElementOtherwiseVertexBuffer ?
						gl::ELEMENT_ARRAY_BUFFER :
						gl::ARRAY_BUFFER,
						gl::STATIC_DRAW),
				[](void* obj, uint32_t size) {
					((gl::VBO*)obj)->Resize(size);
				},
				[](void* obj) {
					delete (gl::VBO*)obj;
				});
	}
	
	AllocatorVBO::AllocatorVBO(const char* debug) {
		Init(
				new std::vector<uint64_t>(),
				[](void* obj, uint32_t size) {
					((std::vector<uint64_t>*)obj)->resize(size);
				},
				[](void* obj) {
					delete (std::vector<uint64_t>*)obj;
				});
	}
	
	AllocatorVBO::AllocatorVBO(void* bufferObject,
			void(*resize)(void*, uint32_t newSize),
			void(*destructor)(void*)) {
		Init(bufferObject, resize, destructor);
	}
	
	void AllocatorVBO::Init(void* bufferObject,
			void(*resize)(void*, uint32_t newSize),
			void(*destructor)(void*)) {
		this->allocated = 0;
		this->bufferObject = bufferObject;
		this->resize = resize;
		this->destructor = destructor;
	}
	
	AllocatorVBO::~AllocatorVBO() {
		destructor(bufferObject);
	}
	
	uint32_t AllocatorVBO::Allocate(uint32_t size) {
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
			if(p.first + p.second < allocated) {
				ReserveAdditional(
						std::max(std::max(size, allocated/2),
							4096u));
			} else {
				ReserveAdditional(
						std::max(std::max(size-p.second,
								allocated), 4096u));
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
		if(freeRanges.size() == 0) {
			freeRanges[pos] = size;
		} else {
			freeRanges[pos] = size;
			auto it = freeRanges.find(pos);
			{
				auto next = it; ++next;
				if(next != freeRanges.end()) {
					if(pos+size == next->first) {
						it->second += next->second;
						freeRanges.erase(next);
						it = freeRanges.find(pos);
					}
				}
			}
			if(it == freeRanges.begin()) { // equivalent to if prev exists
				auto prev = it; --prev;
				if(prev->first + prev->second == it->first) {
					prev->second += size;
					freeRanges.erase(it);
				}
			}
		}
	}
	
	void AllocatorVBO::ReserveAdditional(uint32_t additionalElements) {
		uint32_t prevSize = allocated;
		uint32_t newSize = prevSize+additionalElements;
		if(resize)
			resize(bufferObject, newSize);
		Free(prevSize, additionalElements);
		allocated = newSize;
	}
}

