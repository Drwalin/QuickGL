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

// #include "../../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
// #include "../../../OpenGLWrapper/include/openglwrapper/Shader.hpp"
// 
// #include "../../include/quickgl/util/ManagedSparselyUpdatedVBO.hpp"

namespace qgl {
	
	/*
	template<typename T>
	ManagedSparselyUpdatedVBO<T>::ManagedSparselyUpdatedVBO() {
		vbo = new gl::VBO(sizeof(T), gl::ARRAY_BUFFER, gl::DYNAMIC_DRAW);;
		deltaVbo = new gl::VBO(sizeof(UpdateDataStruct), gl::ARRAY_BUFFER,
				gl::DYNAMIC_DRAW);;
		shader = nullptr;
	}
	
	template<typename T>
	ManagedSparselyUpdatedVBO<T>::~ManagedSparselyUpdatedVBO() {
		Destroy();
	}
	
	template<typename T>
	void ManagedSparselyUpdatedVBO<T>::Init() {
		maxId = 0;
		if(!vbo) {
			vbo = new gl::VBO(sizeof(T), gl::ARRAY_BUFFER, gl::DYNAMIC_DRAW);
		}
		vbo->Init();
		vbo->Resize(100);
		if(!deltaVbo) {
			deltaVbo = new gl::VBO(sizeof(UpdateDataStruct), gl::ARRAY_BUFFER,
					gl::DYNAMIC_DRAW);;
		}
		deltaVbo->Init();
		deltaVbo->Resize(100);
		if(!shader) {
			shader = new gl::Shader();
			const std::string shaderSource = std::string(R"(#version 430 core
const uint ELEMENT_SIZE = )") +
std::to_string(sizeof(T)) + R"(;
struct Data {
	uint data[ELEMENT_SIZE/4];
};
struct DeltaData {
	Data data;
	uint id;
};

layout (std430, binding=1) buffer updateData {
	DeltaData deltaData[];
};
layout (std430, binding=2) buffer dataBuffer {
	Data data[];
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

layout (location=1) uniform uint updateElements;

void main() {
	if(gl_GlobalInvocationID.x >= updateElements)
		return;
	uint id = deltaData[gl_GlobalInvocationID.x].id;
	data[id] = deltaData[gl_GlobalInvocationID.x].data;
}
)";
			shader->Compile(shaderSource);
		}
		deltaData.clear();
	}
	
	template<typename T>
	void ManagedSparselyUpdatedVBO<T>::Destroy() {
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
	
	template<typename T>
	void ManagedSparselyUpdatedVBO<T>::Resize(uint32_t size) {
		vbo->Resize(size);
	}
	
	template<typename T>
	void ManagedSparselyUpdatedVBO<T>::UpdateVBO() {
		if(vbo->GetVertexCount() <= maxId) {
			vbo->Resize(maxId + 100);
		}
		deltaVbo->Update(&deltaData.front(), 0,
				deltaData.size()*sizeof(UpdateDataStruct));
		shader->Use();
		shader->SetInt(1, deltaData.size());
		deltaVbo->BindBufferBase(gl::ARRAY_BUFFER, 1);
		vbo->BindBufferBase(gl::ARRAY_BUFFER, 2);
		shader->DispatchRoundGroupNumbers(deltaData.size(), 1, 1);
		deltaData.clear();
		whereSomethingWasUpdated.clear();
	}
	
	template<typename T>
	void ManagedSparselyUpdatedVBO<T>::SetValue(const T& value, uint32_t id) {
		auto it = whereSomethingWasUpdated.find(id);
		if(it != whereSomethingWasUpdated.end()) {
			deltaData[it->second].value = value;
			deltaData[it->second].id = id;
		} else {
			whereSomethingWasUpdated[id] = deltaData.size();
			deltaData.push_back({value, id});
		}
		maxId = std::max(maxId, id);
	}

	template<typename T>
	uint32_t ManagedSparselyUpdatedVBO<T>::Count() const {
		return vbo->GetVertexCount();
	}
*/
}

