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

#include "../../include/quickgl/util/BufferedVBO.hpp"

#include "../../../OpenGLWrapper/include/openglwrapper/VBO.hpp"

namespace qgl {
	
	BufferedVBO::BufferedVBO(uint32_t vertexSize) : vertexSize(vertexSize) {
		vbo = new gl::VBO(vertexSize, gl::ARRAY_BUFFER, gl::DYNAMIC_DRAW);
		vertices = 0;
	}
	
	BufferedVBO::~BufferedVBO() {
		delete vbo;
	}
	
	void BufferedVBO::Resize(uint32_t vertices) {
		this->vertices = vertices;
		buffer.resize(vertices*sizeof(vertexSize));
		vbo->Resize(vertices);
	}
	
	void BufferedVBO::UpdateVertices(uint32_t vertexStart,
			uint32_t vertexCount) {
		vbo->Update(&buffer.front(), vertexStart*vertexSize,
				vertexCount*vertexSize);
	}
}

