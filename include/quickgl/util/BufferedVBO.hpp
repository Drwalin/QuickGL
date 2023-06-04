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

#ifndef QUICKGL_BUFFERED_VBO_HPP
#define QUICKGL_BUFFERED_VBO_HPP

#include <cinttypes>

#include <vector>

namespace gl {
	class VBO;
}

namespace qgl {
	class BufferedVBO {
	public:
		
		BufferedVBO(uint32_t vertexSize);
		~BufferedVBO();
		
		void Destroy();
		
		inline gl::VBO& Vbo() { return *vbo; }
		
		void Resize(uint32_t vertices);
		
		template<typename T>
		inline T* Elements() { return (T*)(buffer.data()); }
		
		inline uint32_t Count() { return vertices; }
		
		void UpdateVertices(uint32_t vertexStart, uint32_t vertexCount);
		
	protected:
		
		std::vector<uint8_t> buffer;
		gl::VBO* vbo;
		const uint32_t vertexSize;
		uint32_t vertices;
	};
	
	template<typename T>
	class TypedVBO : public BufferedVBO {
	public:
		
		TypedVBO() : BufferedVBO(sizeof(T)) {}
		~TypedVBO() {}
		
		inline T* Elements() { return BufferedVBO::Elements<T>(); }
		inline T& operator[](uint32_t id) {
			if(id >= vertices) {
				Resize(id+1);
			}
			return Elements()[id];
		}
	};
}

#endif

