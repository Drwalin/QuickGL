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

#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VAO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Shader.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/basic_mesh_loader/Mesh.hpp"

#include "../../include/quickgl/MeshManager.hpp"
#include "../../include/quickgl/cameras/Camera.hpp"
#include "../../include/quickgl/util/RenderStageComposer.hpp"

#include "../../include/quickgl/pipelines/PipelineFrustumCuling.hpp"

namespace qgl {
	PipelineFrustumCulling::PipelineFrustumCulling() {
	}
	
	PipelineFrustumCulling::~PipelineFrustumCulling() {
	}
	
	uint32_t PipelineFrustumCulling::GetEntitiesToRender() const {
		return frustumCulledEntitiesCount;
	}
	
	void PipelineFrustumCulling::Initialize() {
		PipelineIdsManagedBase::Initialize();
		
		// init buffer objects
		indirectDrawBuffer = std::make_shared<gl::VBO>(sizeof(uint32_t)*5,
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		indirectDrawBuffer->Init();
		
		frustumCulledFlags = std::make_shared<gl::VBO>(sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		frustumCulledFlags->Init();
		
		frustumCulledIdsBuffer = std::make_shared<gl::VBO>(sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		frustumCulledIdsBuffer->Init();
		
		clippingPlanes = std::make_shared<gl::VBO>(sizeof(float)*4,
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		clippingPlanes->Init();
		
		frustumCulledIdsCountAtomicCounter = std::make_shared<gl::VBO>(sizeof(uint32_t),
				gl::DISPATCH_INDIRECT_BUFFER, gl::DYNAMIC_DRAW);
		frustumCulledIdsCountAtomicCounter->Init();
		const static uint32_t ints[3] = {0, 1, 1};
		frustumCulledIdsCountAtomicCounter->Generate(ints, 3);
		
		frustumCulledIdsCountAtomicCounterAsyncFetch = std::make_shared<gl::VBO>(
				sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		mappedPointerToentitiesCount = (uint32_t*)
			frustumCulledIdsCountAtomicCounterAsyncFetch->InitMapPersistent(
					nullptr, 3,
					gl::MAP_WRITE_BIT | gl::MAP_FLUSH_EXPLICIT_BIT);
		
		// init shaders
		indirectDrawBufferShader = std::make_unique<gl::Shader>();
		if(indirectDrawBufferShader->Compile(INDIRECT_DRAW_BUFFER_COMPUTE_SHADER_SOURCE))
			exit(31);
		
		frustumCullingShader = std::make_unique<gl::Shader>();
		if(frustumCullingShader->Compile(FRUSTUM_CULLING_COMPUTE_SHADER_SOURCE))
			exit(31);
		
		objectsPerInvocation = 16;
		frustumCullingShader->SetUInt(
				frustumCullingShader
					->GetUniformLocation("objectsPerInvocation"),
				objectsPerInvocation);
		
		frustumCullingOnlyShader = std::make_unique<gl::Shader>();
		if(frustumCullingOnlyShader->Compile(FRUSTUM_CULLING_ONLY_COMPUTE_SHADER_SOURCE))
			exit(31);
	}
	
	uint32_t PipelineFrustumCulling::FlushDataToGPU(uint32_t stageId) {
		uint32_t ret = PipelineIdsManagedBase::FlushDataToGPU(stageId);
		if(stageId==0) {
			uint32_t i = indirectDrawBuffer->GetVertexCount();
			while(i < idsManager.CountIds()) {
				i = (i*3)/2 + 100;
			}
			if(i != indirectDrawBuffer->GetVertexCount()) {
				indirectDrawBuffer->Generate(nullptr, i);
				frustumCulledIdsBuffer->Generate(nullptr, i);
				frustumCulledFlags->Generate(nullptr, i);
			}
			
		}
		frustumCulledEntitiesCount = 0;
		frustumCulledIdsCountAtomicCounter
			->Update(&frustumCulledEntitiesCount, 0, sizeof(uint32_t));
		return ret;
	}
	
	void PipelineFrustumCulling::GenerateRenderStages(
			std::vector<Stage>& stages) {
		PipelineIdsManagedBase::GenerateRenderStages(stages);
		
		{
		stages.emplace_back(
			"Updating clipping planes of camera to GPU",
			STAGE_PER_CAMERA,
			[=](std::shared_ptr<Camera> camera) {
				camera->GetClippingPlanes(clippingPlanesValues);
				clippingPlanes->Update(clippingPlanesValues, 0, 5*4*sizeof(float));
			}
		);
		}

		{
		const int32_t FRUSTUM_CULLING_LOCATION_ENTITIES_COUNT =
			frustumCullingOnlyShader->GetUniformLocation("entitiesCount");
		const int32_t FRUSTUM_CULLING_LOCATION_VIEW_MATRIX =
			frustumCullingOnlyShader->GetUniformLocation("cameraInverseTransform");
		
		stages.emplace_back(
			"Performing frustum culling only",
			STAGE_PER_CAMERA,
			[=](std::shared_ptr<Camera> camera) {
				// set visible entities count
				frustumCullingOnlyShader->Use();
				frustumCullingOnlyShader
					->SetUInt(FRUSTUM_CULLING_LOCATION_ENTITIES_COUNT,
							idsManager.CountIds());
				frustumCullingOnlyShader
					->SetMat4(FRUSTUM_CULLING_LOCATION_VIEW_MATRIX,
							camera->GetViewMatrix());

				// bind buffers
				frustumCulledFlags
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
				idsManager.Vbo()
					.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 2);
				transformMatrices.Vbo()
					.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 3);
				clippingPlanes
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 5);
				perEntityMeshInfoBoundingSphere.Vbo()
					.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 6);

				// perform frustum culling
				
				frustumCullingOnlyShader
					->DispatchRoundGroupNumbers(
							(idsManager.CountIds()+31)/32,
							1, 1);
				gl::Shader::Unuse();
			});
		}
		
		{
		const int32_t FRUSTUM_CULLING_LOCATION_ENTITIES_COUNT =
			frustumCullingShader->GetUniformLocation("entitiesCount");

		stages.emplace_back(
			"Performing culled ids construction",
			STAGE_PER_CAMERA,
			[=](std::shared_ptr<Camera> camera) {
				// set visible entities count
				frustumCullingShader->Use();
				frustumCullingShader
					->SetUInt(FRUSTUM_CULLING_LOCATION_ENTITIES_COUNT,
							idsManager.CountIds());

				// bind buffers
				frustumCulledIdsBuffer
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
				idsManager.Vbo()
					.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 2);
				frustumCulledFlags
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 3);
				frustumCulledIdsCountAtomicCounter
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 4);

				// perform frustum culling
				
				frustumCullingShader
					->DispatchRoundGroupNumbers(
							(idsManager.CountIds()+31) /
								32,
							1, 1);
				gl::Shader::Unuse();
				
				frustumCulledIdsCountAtomicCounterAsyncFetch->
					Copy(frustumCulledIdsCountAtomicCounter.get(), 0, 0, 12);
				
				frustumCulledIdsCountAtomicCounterAsyncFetch->
					FlushFromGpuMapPersistentFullRange();
				
				syncFrustumCulledEntitiesCountReadyToFetch.StartFence();
			});
		}

		{
		stages.emplace_back(
			"Fetching count of entities in frustum view to CPU",
			STAGE_PER_CAMERA,
			[this](std::shared_ptr<Camera> camera) {
				// wait for fence
				if(syncFrustumCulledEntitiesCountReadyToFetch.WaitClient(1000000000) == gl::SYNC_TIMEOUT) {
					gl::Finish();
				}
				syncFrustumCulledEntitiesCountReadyToFetch.Destroy();
			
				// fetch number of entities to render after culling
				frustumCulledEntitiesCount = mappedPointerToentitiesCount[0];
			},
			[this](std::shared_ptr<Camera> camera) -> bool {
				return syncFrustumCulledEntitiesCountReadyToFetch.IsDone();
			});
		}

		{
		stages.emplace_back(
			"Generating indirect draw buffer",
			STAGE_PER_CAMERA,
			[=](std::shared_ptr<Camera> camera) {
				// set visible entities count
				indirectDrawBufferShader->Use();
				
				// bind buffers
				frustumCulledIdsCountAtomicCounter
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 4);
				indirectDrawBuffer
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 5);
				perEntityMeshInfo.Vbo()
					.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 6);
				frustumCulledIdsBuffer
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 7);
				
				// generate indirect draw command
				indirectDrawBufferShader->DispatchRoundGroupNumbers(
						frustumCulledEntitiesCount, 1, 1);
				gl::Shader::Unuse();
				
				gl::MemoryBarrier(gl::BUFFER_UPDATE_BARRIER_BIT |
						gl::SHADER_STORAGE_BARRIER_BIT |
						gl::UNIFORM_BARRIER_BIT | gl::COMMAND_BARRIER_BIT);
			});
		}
	}
	
	const char* PipelineFrustumCulling::FRUSTUM_CULLING_ONLY_COMPUTE_SHADER_SOURCE = R"(
#version 450 core

layout (packed, std430, binding=1) writeonly buffer aaa {
	uint frustumCulledEntitiesIds[];
};
layout (packed, std430, binding=2) readonly buffer bbb {
	uint allEntitiesIds[];
};
layout (packed, std430, binding=3) readonly buffer ccc {
	mat4 entitesTransformations[];
};
layout (packed, std430, binding=5) readonly buffer eee {
	vec4 clippingPlanes[];
};
layout (packed, std430, binding=6) readonly buffer fff {
	vec4 meshInfo[];
};

layout (local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

uniform uint entitiesCount;
uniform mat4 cameraInverseTransform;

uint IsInView(uint id) {
	vec3 pos = (
		cameraInverseTransform *
		entitesTransformations[id] *
			vec4(meshInfo[id].xyz, 1)).xyz;
	for(uint i=0; i<5; ++i) {
		float d = dot(clippingPlanes[i].xyz, pos);
		if(d+meshInfo[id].w < clippingPlanes[i].w) {
			return 0;
		}
	}
	return 1;
}

void main() {
	uint inViewCount = 0;
	uint inViewFlags = 0;

	for(uint i=0; i<32; ++i) {
		uint invocationId = gl_GlobalInvocationID.x*32 + i;
		if(invocationId >= entitiesCount)
			break;
		uint isIn = IsInView(allEntitiesIds[invocationId]);
		inViewCount += isIn;
		inViewFlags |= isIn << i;
	}

	frustumCulledEntitiesIds[gl_GlobalInvocationID.x] = inViewFlags;
}
)";
	
	const char* PipelineFrustumCulling::FRUSTUM_CULLING_COMPUTE_SHADER_SOURCE = R"(
#version 450 core

layout (packed, std430, binding=1) writeonly buffer aaa {
	uint frustumCulledEntitiesIds[];
};
layout (packed, std430, binding=2) readonly buffer bbb {
	uint allEntitiesIds[];
};
layout (packed, std430, binding=3) readonly buffer ccc {
	uint frustumCulledEntitiesIdsRead[];
};
layout (packed, std430, binding=4) buffer ddd {
	uint globalAtomicCounter;
};

layout (local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

uniform uint entitiesCount;

shared uint localAtomicCounter;
shared uint commonStartingLocation;

void main() {
	if(gl_LocalInvocationID.x == 0)
		localAtomicCounter = 0;
	barrier();

	uint id = gl_GlobalInvocationID.x;
	uint bitFields = frustumCulledEntitiesIdsRead[id];
	
	uint inViewCount = bitCount(bitFields);
	
	uint localStartingLocation = 0;
	if(inViewCount > 0) // @TODO: this condition can be removed
	                                            // and code still will work:
	                                            // @TODO: check if removign this condition is faster
		localStartingLocation = atomicAdd(localAtomicCounter, inViewCount);
	
	barrier();
	if(gl_LocalInvocationID.x == 0)
		commonStartingLocation = atomicAdd(globalAtomicCounter, localAtomicCounter);
	barrier();

	if(inViewCount == 0)
		return;
	
	uint globalStartingLocation = commonStartingLocation + localStartingLocation;
	
	uint off = globalStartingLocation;
	id = id * 32; 
	for(uint i=0; i<32; ++i, ++id) {
		uint isInView = (bitFields >> i) & 1;
		if(isInView == 1) {
			frustumCulledEntitiesIds[off] = allEntitiesIds[id];
			++off;
		}
	}
}
)";
	
	const char* PipelineFrustumCulling::INDIRECT_DRAW_BUFFER_COMPUTE_SHADER_SOURCE = R"(
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

layout (packed, std430, binding=4) readonly buffer cccaaa {
	uint entitiesCount;
};
layout (packed, std430, binding=5) writeonly buffer aaa {
	DrawElementsIndirectCommand indirectCommands[];
};
layout (packed, std430, binding=6) readonly buffer bbb {
	PerEntityMeshInfo meshInfo[];
};
layout (packed, std430, binding=7) readonly buffer ccc {
	uint visibleEntityIds[];
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

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

