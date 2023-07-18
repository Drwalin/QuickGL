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

#include <memory>

#include "../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../OpenGLWrapper/include/openglwrapper/Shader.hpp"
#include "../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

#include "../include/quickgl/Engine.hpp"
#include "../include/quickgl/util/DeltaVboManager.hpp"

#include "../include/quickgl/IndirectDrawBufferGenerator.hpp"

namespace qgl {
	IndirectDrawBufferGenerator::IndirectDrawBufferGenerator(
			std::shared_ptr<Engine> engine) : engine(engine) {
	}
	
	IndirectDrawBufferGenerator::~IndirectDrawBufferGenerator() {
	}
	
	void IndirectDrawBufferGenerator::Init() {
		// init shaders
		shader = std::make_unique<gl::Shader>();
		if(shader->Compile(INDIRECT_DRAW_BUFFER_COMPUTE_SHADER_SOURCE))
			exit(31);
		
		ENTITIES_COUNT_LOCATION = shader->GetUniformLocation("entitiesCount");
		ENTITIES_OFFSET_LOCATION = shader->GetUniformLocation("entitiesOffset");
	}
	
	void IndirectDrawBufferGenerator::Destroy() {
		engine = nullptr;
		shader->Destroy();
		shader = nullptr;
	}
	
	std::shared_ptr<gl::VBO> IndirectDrawBufferGenerator::Generate(
			gl::VBO& entitiesToRender,
			gl::VBO& meshInfo,
			uint32_t entitiesCount,
			uint32_t entitiesOffset,
			uint32_t& generatedCount) {
		auto vbo = engine->GetDeltaVboManager()->GetNextUpdateVBO();
		generatedCount = std::min<uint32_t>(entitiesCount, vbo->GetVertexCount()*vbo->VertexSize()/20);
		Generate(entitiesToRender, meshInfo, *vbo, entitiesCount, entitiesOffset);
		return vbo;
	}
	
	void IndirectDrawBufferGenerator::Generate(
			gl::VBO& entitiesToRender,
			gl::VBO& meshInfo,
			gl::VBO& indirectDrawBuffer,
			uint32_t entitiesCount,
			uint32_t entitiesOffset) {
		// set visible entities count
		shader->Use();
		
		// bind buffers
		entitiesToRender
			.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
		meshInfo
			.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 2);
		indirectDrawBuffer
			.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 3);
		shader->SetUInt(ENTITIES_COUNT_LOCATION, entitiesCount);
		shader->SetUInt(ENTITIES_OFFSET_LOCATION, entitiesOffset);
		
		// generate indirect draw command
		shader->DispatchRoundGroupNumbers(entitiesCount, 1, 1);
		gl::Shader::Unuse();
		
		gl::MemoryBarrier(gl::BUFFER_UPDATE_BARRIER_BIT |
				gl::SHADER_STORAGE_BARRIER_BIT |
				gl::UNIFORM_BARRIER_BIT | gl::COMMAND_BARRIER_BIT);
	}
	
	const char* IndirectDrawBufferGenerator::INDIRECT_DRAW_BUFFER_COMPUTE_SHADER_SOURCE = R"(
#version 420 core
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_storage_buffer_object : require

struct DrawElementsIndirectCommand {
	uint count;
	uint instanceCount;
	uint firstIndex;
	int  baseVertex;
	uint baseInstance;
};

struct PerEntityMeshInfo {
	uint elementsStart;
	uint elementsCount;
};

layout (packed, std430, binding=1) readonly buffer ccc {
	uint visibleEntityIds[];
};
layout (packed, std430, binding=2) readonly buffer bbb {
	PerEntityMeshInfo meshInfo[];
};
layout (packed, std430, binding=3) writeonly buffer aaa {
	DrawElementsIndirectCommand indirectCommands[];
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

uniform uint entitiesCount;
uniform uint entitiesOffset;

void main() {
	if(gl_GlobalInvocationID.x >= entitiesCount)
		return;
	uint ids = entitiesOffset + gl_GlobalInvocationID.x;
	uint id = visibleEntityIds[ids];
	indirectCommands[ids] = DrawElementsIndirectCommand(
		meshInfo[id].elementsCount,
		1,
		meshInfo[id].elementsStart,
		0,
		id
	);
}
)";
}

