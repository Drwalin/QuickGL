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
#include "../../OpenGLWrapper/include/openglwrapper/Shader.hpp"

#include "../../include/quickgl/util/ManagedSparselyUpdatedVBO.hpp"

namespace qgl {
	UntypedManagedSparselyUpdatedVBO::UntypedManagedSparselyUpdatedVBO(
			uint32_t elementSize) : ELEMENT_SIZE(elementSize),
   			UPDATE_STRUCUTRE_SIZE(ELEMENT_SIZE+sizeof(uint32_t))	{
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
					gl::ARRAY_BUFFER, gl::DYNAMIC_DRAW);;
		}
		deltaVbo->Init();
		deltaVbo->Resize(100);
		if(!shader) {
			shader = new gl::Shader();
			const std::string shaderSource = std::string(R"(#version 450 core
const uint ELEMENT_SIZE = )") +
std::to_string(ELEMENT_SIZE) + R"(;
struct Data {
	uint data[ELEMENT_SIZE/4];
};
struct DeltaData {
	Data data;
	uint id;
};

layout (packed, std430, binding=4) readonly buffer updateData {
	DeltaData deltaData[];
};
layout (packed, std430, binding=5) writeonly buffer dataBuffer {
	Data data[];
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

uniform uint updateElements;

void main() {
	if(gl_GlobalInvocationID.x >= updateElements)
		return;
	uint id = deltaData[gl_GlobalInvocationID.x].id;
	data[id] = deltaData[gl_GlobalInvocationID.x].data;
}
)";
			shader->Compile(shaderSource);
			shaderDeltaCommandsLocation = shader->GetUniformLocation("updateElements");
		}
		deltaData.clear();
	}
	
	void UntypedManagedSparselyUpdatedVBO::Destroy() {
		if(vbo) {
			delete vbo;
			vbo = nullptr;
		}
		if(deltaVbo) {
			delete deltaVbo;
			deltaVbo = nullptr;
		}
		if(shader) {
			delete shader;
			shader = nullptr;
		}
		deltaData.clear();
	}
	
	void UntypedManagedSparselyUpdatedVBO::Resize(uint32_t size) {
		vbo->Resize(size);
	}
	
	void UntypedManagedSparselyUpdatedVBO::UpdateVBO() {
		if(vbo->GetVertexCount() <= maxId) {
			vbo->Resize(maxId + 100);
		}
		deltaData.swap(deltaDataGPU);
		deltaData.clear();
		if(deltaDataGPU.size() != 0) {
			deltaVbo->Update(&deltaDataGPU.front(), 0,
					deltaDataGPU.size()*UPDATE_STRUCUTRE_SIZE);
			shader->Use();
			shader->SetUInt(shaderDeltaCommandsLocation, deltaDataGPU.size());
			deltaVbo->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 4);
			vbo->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 5);
			shader->DispatchRoundGroupNumbers(deltaDataGPU.size(), 1, 1);
			whereSomethingWasUpdated.clear();
		}
	}
	
	void UntypedManagedSparselyUpdatedVBO::SetValue(const void* value, uint32_t id) {
		auto it = whereSomethingWasUpdated.find(id);
		uint32_t p = deltaData.size();
		if(it != whereSomethingWasUpdated.end()) {
			p = it->second*UPDATE_STRUCUTRE_SIZE;
		} else {
			deltaData.resize(p+UPDATE_STRUCUTRE_SIZE);
		}
		memcpy(&(deltaData[p]), value, ELEMENT_SIZE);
		*(uint32_t*)&(deltaData[p+ELEMENT_SIZE]) = id;
		maxId = std::max(maxId, id);
	}

	uint32_t UntypedManagedSparselyUpdatedVBO::Count() const {
		return vbo->GetVertexCount();
	}
}

