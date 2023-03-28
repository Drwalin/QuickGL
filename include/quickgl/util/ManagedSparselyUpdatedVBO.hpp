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

#ifndef QUICKGL_MANAGED_SPARSELY_UPDATED_VBO_HPP
#define QUICKGL_MANAGED_SPARSELY_UPDATED_VBO_HPP

#include <vector>
#include <unordered_map>

#include "BufferedVBO.hpp"

namespace gl {
	class Shader;
	class VBO;
}

#include "../../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../../OpenGLWrapper/include/openglwrapper/Shader.hpp"

namespace qgl {
	template<typename T>
	class ManagedSparselyUpdatedVBO final {
	public:
		
		ManagedSparselyUpdatedVBO();
		~ManagedSparselyUpdatedVBO();
		
		void Init();
		void Destroy();
		
		void Resize(uint32_t size);
		
		inline gl::VBO& Vbo() { return *vbo; }
		
		void UpdateVBO();
		
		void SetValue(const T& value, uint32_t id);
		
		uint32_t Count() const;
		
	private:
		
		struct UpdateDataStruct {
			T value;
			uint32_t id;
		};
		
		gl::VBO* vbo;
		gl::VBO* deltaVbo;
		gl::Shader* shader;
		
		uint32_t maxId;
		int32_t shaderDeltaCommandsLocation;
		
		std::vector<UpdateDataStruct> deltaData;
		std::unordered_map<uint32_t, uint32_t> whereSomethingWasUpdated;
	};
	
	
	
	
	
	
	
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
			const std::string shaderSource = std::string(R"(#version 450 core
const uint ELEMENT_SIZE = )") +
std::to_string(sizeof(T)) + R"(;
struct Data {
	uint data[ELEMENT_SIZE/4];
};
struct DeltaData {
	Data data;
	uint id;
};

layout (std430, binding=4) buffer updateData {
	DeltaData deltaData[];
};
layout (std430, binding=5) buffer dataBuffer {
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
	GL_CHECK_PUSH_ERROR;
		if(vbo->GetVertexCount() <= maxId) {
			vbo->Resize(maxId + 100);
		}
	GL_CHECK_PUSH_ERROR;
		if(deltaData.size() != 0) {
			deltaVbo->Update(&deltaData.front(), 0,
					deltaData.size()*sizeof(UpdateDataStruct));
		GL_CHECK_PUSH_ERROR;
			shader->Use();
		GL_CHECK_PUSH_ERROR;
			shader->SetUInt(shaderDeltaCommandsLocation, deltaData.size());
		GL_CHECK_PUSH_ERROR;
			deltaVbo->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 4);
		GL_CHECK_PUSH_ERROR;
			vbo->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 5);
		GL_CHECK_PUSH_ERROR;
			shader->DispatchRoundGroupNumbers(deltaData.size(), 1, 1);
		GL_CHECK_PUSH_ERROR;
			deltaData.clear();
		GL_CHECK_PUSH_ERROR;
			whereSomethingWasUpdated.clear();
		}
	GL_CHECK_PUSH_ERROR;
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
}

#endif

// #include "../../../src/util/ManagedSparselyUpdatedVBO.cpp"

