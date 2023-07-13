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

#include <cstring>

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VAO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Shader.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

#include "../../include/quickgl/util/ManagedSparselyUpdatedVBO.hpp"

namespace qgl {
	UntypedManagedSparselyUpdatedVBO::UntypedManagedSparselyUpdatedVBO(
			uint32_t elementSize) :
			ELEMENT_SIZE((elementSize+3)-((elementSize+3)&3)),
   			UPDATE_STRUCUTRE_SIZE(ELEMENT_SIZE+sizeof(uint32_t)) {
		vbo = nullptr;
		deltaVbo = nullptr;
		shader = nullptr;
	}
	
	UntypedManagedSparselyUpdatedVBO::~UntypedManagedSparselyUpdatedVBO() {
		Destroy();
	}
	
	void UntypedManagedSparselyUpdatedVBO::Init() {
		maxId = 0;
		if(!vbo) {
			vbo = new gl::VBO(ELEMENT_SIZE, gl::ARRAY_BUFFER, gl::DYNAMIC_DRAW);
		}
		vbo->Init();
		vbo->Resize(100);
		if(!deltaVbo) {
			deltaVbo = new gl::VBO(UPDATE_STRUCUTRE_SIZE,
					gl::ARRAY_BUFFER, gl::DYNAMIC_DRAW);
		}
		deltaVbo->Init();
		deltaVbo->Resize(16*1024);
		if(!shader) {
			shader = new gl::Shader();
			const std::string shaderSource = std::string(R"(#version 450 core
const uint ELEMENT_SIZE = )") + std::to_string(ELEMENT_SIZE) + R"(;
struct Data {
	uint data[ELEMENT_SIZE/4];
};
struct DeltaData {
	Data data;
	uint id;
};

layout (packed, std430, binding=1) readonly buffer updateData {
	DeltaData deltaData[];
};
layout (packed, std430, binding=2) writeonly buffer dataBuffer {
	Data data[];
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

uniform uint updateElements;

void main() {
	uint self = gl_GlobalInvocationID.x;
	if(self >= updateElements)
		return;
	uint id = deltaData[self].id;
	data[id] = deltaData[self].data;
})";
			if(shader->Compile(shaderSource))
				exit(31);
			shaderDeltaCommandsLocation =
				shader->GetUniformLocation("updateElements");
		}
		deltaData.clear();
	}
	
	void UntypedManagedSparselyUpdatedVBO::Destroy() {
		if(shader) {
			gl::Finish();
			delete shader;
			shader = nullptr;
		}
		if(vbo) {
			gl::Finish();
			delete vbo;
			vbo = nullptr;
		}
		if(deltaVbo) {
			gl::Finish();
			delete deltaVbo;
			deltaVbo = nullptr;
		}
		deltaData.clear();
	}
	
	void UntypedManagedSparselyUpdatedVBO::Resize(uint32_t size) {
		vbo->Resize(size);
	}
	
	void UntypedManagedSparselyUpdatedVBO::UpdateVBO() {
		if(deltaData.size() == 0)
			return;
		if(vbo->GetVertexCount() <= maxId) {
			vbo->Resize((maxId*3)/2+100);
		}
		const uint32_t vs = UPDATE_STRUCUTRE_SIZE;
		shader->Use();
		for(uint32_t i=0; i<deltaData.size()/vs; i+=deltaVbo->GetVertexCount()) {
			const uint32_t count = std::min<uint32_t>(deltaVbo->GetVertexCount(),
					deltaData.size()/vs - i);
			deltaVbo->Update(deltaData.data()+(i*vs), 0, count*vs);
			//TODO: check if memory barrier is needed
 			gl::MemoryBarrier(gl::MemoryBarriers::BUFFER_UPDATE_BARRIER_BIT);
			shader->SetUInt(shaderDeltaCommandsLocation, count);
			deltaVbo->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
			vbo->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 2);
			shader->DispatchRoundGroupNumbers(count, 1, 1);
		}
		gl::Shader::Unuse();
		whereSomethingWasUpdated.clear();
		deltaData.clear();
	}
	
	void UntypedManagedSparselyUpdatedVBO::SetValue(const void* value,
			uint32_t id) {
		auto it = whereSomethingWasUpdated.find(id);
		uint32_t p = deltaData.size();
		if(it != whereSomethingWasUpdated.end()) {
			p = it->second;
		} else {
			deltaData.resize(p+UPDATE_STRUCUTRE_SIZE);
			whereSomethingWasUpdated[id] = p;
		}
		memcpy(&(deltaData[p]), value, ELEMENT_SIZE);
		*(uint32_t*)&(deltaData[p+ELEMENT_SIZE]) = id;
		maxId = std::max(maxId, id);
	}

	uint32_t UntypedManagedSparselyUpdatedVBO::Count() const {
		return vbo->GetVertexCount();
	}
}

