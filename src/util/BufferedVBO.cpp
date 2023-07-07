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

#include <cstdio>

#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"

#include "../../include/quickgl/util/BufferedVBO.hpp"

namespace qgl {
	
	BufferedVBO::BufferedVBO(uint32_t vertexSize) : vertexSize(vertexSize) {
		vbo = new gl::VBO(vertexSize, gl::ARRAY_BUFFER, gl::DYNAMIC_DRAW);
		vertices = 0;
	}
	
	BufferedVBO::~BufferedVBO() {
		Destroy();
	}
	
	void BufferedVBO::Destroy() {
		if(vbo) {
			fflush(stdout);
			gl::Finish();
			vbo->Destroy();
			delete vbo;
			vbo = nullptr;
		}
	}
	
	void BufferedVBO::Resize(uint32_t vertices) {
	GL_CHECK_PUSH_ERROR;
		if(this->vertices == 0) {
			vbo->Generate(nullptr, 1);
		}
	GL_CHECK_PUSH_ERROR;
		this->vertices = vertices;
	GL_CHECK_PUSH_ERROR;
		buffer.resize(vertices*vertexSize);
	GL_CHECK_PUSH_ERROR;
		vbo->Resize(vertices);
	GL_CHECK_PUSH_ERROR;
	}
	
	void BufferedVBO::UpdateVertices(uint32_t vertexStart,
			uint32_t vertexCount) {
		vbo->Update(buffer.data()+vertexStart*vertexSize,
				vertexStart*vertexSize, vertexCount*vertexSize);
	}
}

