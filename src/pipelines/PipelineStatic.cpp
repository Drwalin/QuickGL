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

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VAO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Shader.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/basic_mesh_loader/Mesh.hpp"

#include "../../include/quickgl/MeshManager.hpp"
#include "../../include/quickgl/cameras/Camera.hpp"

#include "../../include/quickgl/pipelines/PipelineStatic.hpp"

namespace qgl {
	PipelineStatic::PipelineStatic() {
	}
	
	PipelineStatic::~PipelineStatic() {
	}
	
	uint32_t PipelineStatic::CreateEntity() {
		uint32_t id = PipelineIdsManagedBase::CreateEntity();
		if(id >= vboIndirectDrawBuffer->GetVertexCount()) {
			vboIndirectDrawBuffer->Generate(nullptr, id+100);
		}
		return id;
	}
	
	void PipelineStatic::Initialize() {
		PipelineIdsManagedBase::Initialize();
		vboIndirectDrawBuffer = std::make_shared<gl::VBO>(sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		vboIndirectDrawBuffer->Init();
		
		// init shader
		renderShader = std::make_unique<gl::Shader>();
		renderShader->Compile(VERTEX_SHADER_SOURCE, "", FRAGMENT_SHADER_SOURCE);
		generateIndirectDrawBufferShader = std::make_unique<gl::Shader>();
		generateIndirectDrawBufferShader->Compile(INDIRECT_DRAW_BUFFER_COMPUTE_SHADER_SOURCE);
		
		// init vao
		vao = std::make_unique<gl::VAO>(gl::TRIANGLES);
		vao->Init();
		gl::VBO& vbo = meshManager->GetVBO();
		vbo.Init();
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_pos"), 3, gl::FLOAT, false, 0, 0);
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_color"), 4, gl::UNSIGNED_BYTE, true, 12, 0);
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_normal"), 4, gl::BYTE, true, 16, 0);
		
		// init model matrix
		gl::VBO& modelVbo = transformMatrices.Vbo();
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+0, 4, gl::FLOAT, false, 0, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+1, 4, gl::FLOAT, false, 16, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+2, 4, gl::FLOAT, false, 32, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+3, 4, gl::FLOAT, false, 48, 1);
		vao->BindElementBuffer(meshManager->GetEBO(), gl::UNSIGNED_INT);
		
		// get shader uniform locations
		projectionViewLocation =
			renderShader->GetUniformLocation("projectionView");
		entitesCountToRenderLocation =
			generateIndirectDrawBufferShader
			->GetUniformLocation("entitiesCount");
	}
	
	uint32_t PipelineStatic::DrawStage(std::shared_ptr<Camera> camera,
			uint32_t stageId) {
		switch(stageId) {
			case 0: {
				// set visible entities count
				generateIndirectDrawBufferShader->Use();
				generateIndirectDrawBufferShader
					->SetUInt(entitesCountToRenderLocation,
							idsManager.CountIds());
				
				glMemoryBarrier(GL_ALL_BARRIER_BITS);
				
				// bind buffers
				vboIndirectDrawBuffer
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 5);
				idsManager.Vbo().BindBufferBase(gl::SHADER_STORAGE_BUFFER, 6);
				perEntityMeshInfo.Vbo()
					.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 7);
				
				// generate indirect draw command buffer
				generateIndirectDrawBufferShader
					->DispatchRoundGroupNumbers(idsManager.CountIds(), 1, 1);
				} return 2;
				
			case 1: {
				// draw with indirect draw buffer
					glMemoryBarrier(GL_ALL_BARRIER_BITS);
					renderShader->Use();
					glm::mat4 pv = camera->GetPerspectiveMatrix()
						* camera->GetViewMatrix();
					renderShader->SetMat4(projectionViewLocation, pv);
					vao->BindIndirectBuffer(*vboIndirectDrawBuffer);
					vao->DrawMultiElementsIndirect(NULL, idsManager.CountIds());
				} return 1;
			
			case 2:
				
				return 0;
		}
		return 0;
	}
	
	
	void PipelineStatic::FlushDataToGPU() {
		PipelineIdsManagedBase::FlushDataToGPU();
		if(vboIndirectDrawBuffer->GetVertexCount()
				< idsManager.CountIds()) {
			vboIndirectDrawBuffer->Resize(idsManager.CountIds());
		}
	}
	
	
	
	std::shared_ptr<MeshManager> PipelineStatic::CreateMeshManager() {
		static constexpr uint32_t stride
			= 3*sizeof(float)   // pos
			+ 4*sizeof(uint8_t) // color
			+ 4*sizeof(uint8_t) // normal
			;
		
		return std::make_shared<MeshManager>(stride,
			[](std::vector<uint8_t>& buffer, uint32_t offset,
					gl::BasicMeshLoader::Mesh* mesh){
				
				mesh->ExtractPos<float>(offset, buffer, 0, stride,
						gl::BasicMeshLoader::ConverterFloatPlain<float, 3>);
				
				mesh->ExtractColor<uint8_t>(offset, buffer, 12, stride,
						gl::BasicMeshLoader::ConverterIntPlainClampScale<uint8_t, 255, 0, 255, 4>);
				
				mesh->ExtractNormal(offset, buffer, 16, stride,
						gl::BasicMeshLoader::ConverterIntNormalized<uint8_t, 127, 3>);
			});
	}
	
	const char* PipelineStatic::VERTEX_SHADER_SOURCE = R"(
#version 450 core

in vec3 in_pos;
in vec4 in_color;
in vec3 in_normal;

in mat4 model;

uniform mat4 projectionView;


out vec4 color;
out vec3 normal;
out vec4 pos;

void main() {
	gl_Position = pos = projectionView * model * vec4(in_pos, 1);
	normal = normalize((model * vec4(in_normal, 0)).xyz);
	color = in_color;
}
)";
	
	const char* PipelineStatic::FRAGMENT_SHADER_SOURCE = R"(
#version 450 core

in vec4 color;
in vec3 normal;
in vec4 pos;

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 NormalColor;
layout(location = 2) out vec4 PosColor;

void main() {
	FragColor = color;
	NormalColor = vec4(normal, 0);
	PosColor = pos;
}
)";
		
	const char* PipelineStatic::INDIRECT_DRAW_BUFFER_COMPUTE_SHADER_SOURCE = R"(
#version 450 core

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

layout (packed, std430, binding=5) writeonly buffer aaa {
	DrawElementsIndirectCommand indirectCommands[];
};
layout (packed, std430, binding=7) readonly buffer bbb {
	PerEntityMeshInfo meshInfo[];
};
layout (packed, std430, binding=6) readonly buffer ccc {
	uint visibleEntityIds[];
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

uniform uint entitiesCount;

void main() {
	if(gl_GlobalInvocationID.x >= entitiesCount)
		return;
	uint id = visibleEntityIds[gl_GlobalInvocationID.x];
	indirectCommands[gl_GlobalInvocationID.x] = DrawElementsIndirectCommand(
		meshInfo[id].elementsCount,
		1,
		meshInfo[id].elementsStart,
		0,
		id
	);
}
)";
}

