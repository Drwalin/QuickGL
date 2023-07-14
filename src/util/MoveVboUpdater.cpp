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

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Shader.hpp"

#include "../../include/quickgl/util/DeltaVboManager.hpp"
#include "../../include/quickgl/Engine.hpp"

#include "../../include/quickgl/util/MoveVboUpdater.hpp"

namespace qgl {
		
	MoveVboUpdater::MoveVboUpdater(std::shared_ptr<Engine> engine, uint32_t bytes) :
		BYTES(bytes), engine(engine)  {
	}
	
	MoveVboUpdater::~MoveVboUpdater() {
		Destroy();
	}
	
	void MoveVboUpdater::Init() {
		if(BYTES % 4) {
			throw "qgl::MoveVboUpdater::Init() cannot initialize with BYTES not dividible by 4.";
		}
		
		shader = std::make_shared<gl::Shader>();
		
		const std::string shaderSource = std::string(R"(#version 450 core
const uint ELEMENT_SIZE = )") + std::to_string(BYTES) + R"(;
struct Data {
	uint data[ELEMENT_SIZE/4];
};
struct DeltaData {
	uint from;
	uint to;
};

layout (packed, std430, binding=1) readonly buffer updateData {
	DeltaData deltaData[];
};
layout (packed, std430, binding=2) buffer dataBuffer {
	Data data[];
};

layout (local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

uniform uint updateElementsCount;

void main() {
	uint self = gl_GlobalInvocationID.x;
	if(self >= updateElementsCount)
		return;
	DeltaData delta = deltaData[self];
	data[delta.to] = data[delta.from];
})";
		
		shader->Compile(shaderSource);
		updateElementsCountLocation = shader
			->GetUniformLocation("updateElementsCount");
	}
	
	void MoveVboUpdater::Destroy() {
		shader->Destroy();
		shader = nullptr;
	}
	
	void MoveVboUpdater::Update(gl::VBO* vbo, const PairMove* data, uint32_t elements) {
		shader->Use();
		vbo->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 2);
		for(uint32_t i=0; i<elements;) {
			auto deltaVbo = engine->GetDeltaVboManager()->GetNextUpdateVBO();
			const uint32_t elem = std::min<uint32_t>(
					deltaVbo->GetVertexCount()/sizeof(PairMove), elements-i);
			
			deltaVbo->Update(data+i, 0, elem*sizeof(PairMove));
			gl::MemoryBarrier(gl::BUFFER_UPDATE_BARRIER_BIT);
			
			shader->SetUInt(updateElementsCountLocation, elem);
			deltaVbo->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
			shader->DispatchRoundGroupNumbers(elem, 1, 1);
			
			i += elem;
		}
		shader->Unuse();
	}
	
	void MoveVboUpdater::Update(gl::VBO* vbo, gl::VBO* deltaVbo, uint32_t elements) {
		shader->Use();
		shader->SetUInt(updateElementsCountLocation, elements);
		deltaVbo->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
		vbo->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 2);
		shader->DispatchRoundGroupNumbers(elements, 1, 1);
		shader->Unuse();
	}
		


	MoveVboManager::MoveVboManager(std::shared_ptr<Engine> engine) : engine(engine) {
	}

	MoveVboManager::~MoveVboManager() {
		Destroy();
	}
	
	void MoveVboManager::Destroy() {
		for(auto u : updatersByElementSizeSize) {
			u.second->Destroy();
		}
		updatersByElementSizeSize.clear();
	}
	
	std::shared_ptr<MoveVboUpdater> MoveVboManager::GetByObjectSize(
			uint32_t bytes) {
		if(bytes % 4) {
			throw "qgl::MoveVboManager::GetByObjectSize() cannot initialize with BYTES not dividible by 4.";
		}
		std::shared_ptr<MoveVboUpdater>& updater
			= updatersByElementSizeSize[bytes];
		if(updater == nullptr) {
			updater = std::make_shared<MoveVboUpdater>(engine, bytes);
			updater->Init();
		}
		return updater;
	}
	
	void MoveVboManager::Update(gl::VBO* vbo,
			const MoveVboUpdater::PairMove* data, uint32_t elements,
			uint32_t elementSize) {
		auto updater = GetByObjectSize(elementSize);
		updater->Update(vbo, data, elements);
	}
	
	void MoveVboManager::Update(gl::VBO* vbo, gl::VBO* deltaVbo, uint32_t elements,
			uint32_t elementSize) {
		auto updater = GetByObjectSize(elementSize);
		updater->Update(vbo, deltaVbo, elements);
	}
}

