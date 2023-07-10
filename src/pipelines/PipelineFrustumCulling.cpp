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
		
		frustumCulledIdsBuffer = std::make_shared<gl::VBO>(sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		frustumCulledIdsBuffer->Init();
		
		clippingPlanes = std::make_shared<gl::VBO>(sizeof(float)*4,
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		clippingPlanes->Init();
		
		
		
		areInView = std::make_shared<gl::VBO>(
				sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		reduceOffsets1 = std::make_shared<gl::VBO>(
				sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		reduceOffsets2 = std::make_shared<gl::VBO>(
				sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		reduceOffsets3 = std::make_shared<gl::VBO>(
				sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		reduceOffsets4 = std::make_shared<gl::VBO>(
				sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		reduceCounts1 = std::make_shared<gl::VBO>(
				sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		reduceCounts2 = std::make_shared<gl::VBO>(
				sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		reduceCounts3 = std::make_shared<gl::VBO>(
				sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		reduceCounts3fetch = std::make_shared<gl::VBO>(
				sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		
		areInView->Init(32*4096);
		reduceOffsets1->Init(32*4096);
		reduceOffsets2->Init(32*4096);
		reduceOffsets3->Init(32*4096);
		reduceOffsets4->InitMapPersistent(nullptr, 4096,
				gl::MAP_WRITE_BIT | gl::MAP_FLUSH_EXPLICIT_BIT);
		reduceCounts1->Init(32*4096);
		reduceCounts2->Init(32*4096);
		reduceCounts3->Init(4096);
		reduceCounts3fetch->InitMapPersistent(nullptr, 4096,
				gl::MAP_WRITE_BIT | gl::MAP_FLUSH_EXPLICIT_BIT);
		
		// init shaders
		indirectDrawBufferShader = std::make_unique<gl::Shader>();
		if(indirectDrawBufferShader->Compile(INDIRECT_DRAW_BUFFER_COMPUTE_SHADER_SOURCE))
			exit(31);
		
		frustumCullingShader = std::make_unique<gl::Shader>();
		if(frustumCullingShader->Compile(FRUSTUM_CULLING_COMPUTE_SHADER_SOURCE))
			exit(31);
		
		sumReduceShader = std::make_unique<gl::Shader>();
		if(sumReduceShader->Compile(SUM_REDUCE_COMPUTE_SHADER_SOURCE))
			exit(31);
		
		sumReduceShader->SetUInt(
				sumReduceShader->GetUniformLocation("reduceCount"),
				REDUCE_SIZE);
		
		sumReduceReconstructShader = std::make_unique<gl::Shader>();
		if(sumReduceReconstructShader->Compile(SUM_REDUCE_RECONSTRUCT_COMPUTE_SHADER_SOURCE))
			exit(31);
		
		sumReduceReconstructShader->SetUInt(
				sumReduceReconstructShader->GetUniformLocation("reduceCount"),
				REDUCE_SIZE);
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
			}
			
		}
		frustumCulledEntitiesCount = 0;
		if(reduceOffsets1->GetVertexCount() < GetEntitiesCount()) {
			reduceOffsets1->Resize(GetEntitiesCount());
			areInView->Resize(GetEntitiesCount());
			if(reduceOffsets2->GetVertexCount()
					< ((GetEntitiesCount()+REDUCE_SIZE-1)/REDUCE_SIZE)) {
				reduceOffsets2->Resize((GetEntitiesCount()+REDUCE_SIZE-1)/REDUCE_SIZE);
				reduceCounts1->Resize((GetEntitiesCount()+REDUCE_SIZE-1)/REDUCE_SIZE);
			}
		}
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
			frustumCullingShader->GetUniformLocation("entitiesCount");
		const int32_t FRUSTUM_CULLING_LOCATION_VIEW_MATRIX =
			frustumCullingShader->GetUniformLocation("cameraInverseTransform");

		stages.emplace_back(
			"Performing frustum culling",
			STAGE_PER_CAMERA,
			[=](std::shared_ptr<Camera> camera) {
				// set visible entities count
				frustumCullingShader->Use();
				frustumCullingShader
					->SetUInt(FRUSTUM_CULLING_LOCATION_ENTITIES_COUNT,
							idsManager.CountIds());
				frustumCullingShader
					->SetMat4(FRUSTUM_CULLING_LOCATION_VIEW_MATRIX,
							camera->GetViewMatrix());

				// bind buffers
				areInView
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
				idsManager.Vbo()
					.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 2);
				transformMatrices.Vbo()
					.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 3);
				clippingPlanes
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 4);
				perEntityMeshInfoBoundingSphere.Vbo()
					.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 5);

				// perform frustum culling
				frustumCullingShader
					->DispatchRoundGroupNumbers(idsManager.CountIds(), 1, 1);
				
				gl::Shader::Unuse();
			});
		}

		{
		const int32_t COUNT_IN_LOCATION =
			sumReduceShader->GetUniformLocation("countIn");
		const int32_t COUNT_OUT_LOCATION =
			sumReduceShader->GetUniformLocation("countOut");

		stages.emplace_back(
			"Frustum culling count reduce 1",
			STAGE_PER_CAMERA,
			[=](std::shared_ptr<Camera> camera) {
				sumReduceShader->Use();
				sumReduceShader->SetUInt(COUNT_IN_LOCATION,
							idsManager.CountIds());
				sumReduceShader->SetUInt(COUNT_OUT_LOCATION,
							(idsManager.CountIds()+REDUCE_SIZE-1)/REDUCE_SIZE);

				// bind buffers
				areInView
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
				reduceOffsets1
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 2);
				reduceCounts1
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 3);

				// perform sum reduce
				sumReduceShader
					->DispatchRoundGroupNumbers(idsManager.CountIds(), 1, 1);
				
				gl::Shader::Unuse();
			});

		stages.emplace_back(
			"Frustum culling count reduce 2",
			STAGE_PER_CAMERA,
			[=](std::shared_ptr<Camera> camera) {
				sumReduceShader->Use();
				sumReduceShader->SetUInt(COUNT_IN_LOCATION,
							(idsManager.CountIds()+REDUCE_SIZE-1)/REDUCE_SIZE);
				sumReduceShader->SetUInt(COUNT_OUT_LOCATION,
							(((idsManager.CountIds()+REDUCE_SIZE-1)/REDUCE_SIZE)+REDUCE_SIZE-1)/REDUCE_SIZE);

				// bind buffers
				reduceCounts1
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
				reduceOffsets2
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 2);
				reduceCounts2
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 3);

				// perform sum reduce
				sumReduceShader
					->DispatchRoundGroupNumbers(
							(idsManager.CountIds()+REDUCE_SIZE-1)/REDUCE_SIZE,
							1, 1);
				
				gl::Shader::Unuse();
			});

		stages.emplace_back(
			"Frustum culling count reduce 3",
			STAGE_PER_CAMERA,
			[=](std::shared_ptr<Camera> camera) {
				// perform reduce
				sumReduceShader->Use();
				sumReduceShader->SetUInt(COUNT_IN_LOCATION,
							(((idsManager.CountIds()+REDUCE_SIZE-1)/REDUCE_SIZE)+REDUCE_SIZE-1)/REDUCE_SIZE);
				sumReduceShader->SetUInt(COUNT_OUT_LOCATION,
							(((((idsManager.CountIds()+REDUCE_SIZE-1)/REDUCE_SIZE)+REDUCE_SIZE-1)/REDUCE_SIZE)+REDUCE_SIZE-1)/REDUCE_SIZE);

				// bind buffers
				reduceCounts2
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
				reduceOffsets3
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 2);
				reduceCounts3
					->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 3);

				sumReduceShader
					->DispatchRoundGroupNumbers(
							(((idsManager.CountIds()+REDUCE_SIZE-1)/REDUCE_SIZE)+REDUCE_SIZE-1)/REDUCE_SIZE,
							1, 1);
				
				gl::Shader::Unuse();
				
				reduceCounts3fetch->Copy(reduceCounts3.get(), 0, 0, reduceCounts3fetch->GetVertexCount()*sizeof(uint32_t));
				reduceCounts3fetch->FlushToGpuMapPersistentFullRange();
				
				syncFrustumCulledEntitiesCountReadyToFetch.StartFence();
			});

		stages.emplace_back(
			"Fetching count entities in view, reconstructing culled ids buffer",
			STAGE_PER_CAMERA,
			[this](std::shared_ptr<Camera> camera) {
				// wait for fence
				if(syncFrustumCulledEntitiesCountReadyToFetch.WaitClient(1000000000) == gl::SYNC_TIMEOUT) {
					gl::Finish();
				}
				syncFrustumCulledEntitiesCountReadyToFetch.Destroy();
				
				uint32_t countSrc = 
						(((((idsManager.CountIds()+REDUCE_SIZE-1)/REDUCE_SIZE)+REDUCE_SIZE-1)/REDUCE_SIZE)+REDUCE_SIZE-1)/REDUCE_SIZE;
				
				uint32_t *reduceCountPtr = (uint32_t*)reduceCounts3fetch->GetMappedPointer();
				
				uint32_t *offset = (uint32_t*)reduceOffsets4->GetMappedPointer();
				frustumCulledEntitiesCount = 0;
				
				for(int i=0; i<countSrc; ++i) {
					offset[i] = frustumCulledEntitiesCount;
					frustumCulledEntitiesCount += reduceCountPtr[i];
				}
				
				gl::MemoryBarrier(gl::CLIENT_MAPPED_BUFFER_BARRIER_BIT);
				
				// reconstruct ids buffer
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
	
	const char* PipelineFrustumCulling::FRUSTUM_CULLING_COMPUTE_SHADER_SOURCE = R"(
#version 450 core

layout (packed, std430, binding=1) writeonly buffer aaa {
	uint isInViewOneOrZero[];
};
layout (packed, std430, binding=2) readonly buffer bbb {
	uint allEntitiesIds[];
};
layout (packed, std430, binding=3) readonly buffer ccc {
	mat4 entitesTransformations[];
};
layout (packed, std430, binding=4) readonly buffer eee {
	vec4 clippingPlanes[];
};
layout (packed, std430, binding=5) readonly buffer fff {
	vec4 meshInfo[];
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

uniform uint entitiesCount;
uniform mat4 cameraInverseTransform;

uint IsInView(uint id) {
	if(id < entitiesCount) {
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
	return 0;
}

void main() {
	uint id = gl_GlobalInvocationID.x;
	if(id >= entitiesCount) {
		return;
	}
	isInViewOneOrZero[id] = IsInView(id);
}
)";
	
	const char* PipelineFrustumCulling::SUM_REDUCE_COMPUTE_SHADER_SOURCE = R"(
layout (packed, std430, binding=1) readonly buffer aaa {
	uint inCounts[];
};
layout (packed, std430, binding=2) writeonly buffer bbb{
	uint outOffsets[];
};
layout (packed, std430, binding=3) writeonly buffer ccc {
	uint outCounts[];
};

uniform uint countIn;
uniform uint countOut;
uniform uint reduceCount;

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main() {
	const uint outId = gl_GlobalInvocationID.x;
	const uint startId = outId * reduceCount;
	if(startId >= countIn)
		return;
	const uint endId = min(startId + reduceCount, countIn);
	uint count = 0;
	for(uint id=startId; id<endId; ++id) {
		outOffsets[id] = count;
		count += inCounts[id];
	}
	if(startId < countIn) {
		outCounts[outId] = count;
	}
}
)"; 
	
	const char* PipelineFrustumCulling::SUM_REDUCE_RECONSTRUCT_COMPUTE_SHADER_SOURCE = R"(
layout (packed, std430, binding=1) readonly buffer aaa {
	uint areInView[];
};
layout (packed, std430, binding=) readonly buffer {
	uint reduceOffsets1[];
};
layout (packed, std430, binding=) readonly buffer {
	uint reduceOffsets2[];
};
layout (packed, std430, binding=) readonly buffer {
	uint reduceOffsets3[];
};
layout (packed, std430, binding=) readonly buffer {
	uint reduceOffsets4[];
};
layout (packed, std430, binding=) readonly buffer {
	uint reduceCounts1[];
};
layout (packed, std430, binding=) readonly buffer {
	uint reduceCounts2[];
};
layout (packed, std430, binding=) readonly buffer {
	uint reduceCounts3[];
};

layout (packed, std430, binding=) readonly buffer {
	uint [];
};
layout (packed, std430, binding=) readonly buffer {
	uint [];
};
layout (packed, std430, binding=) readonly buffer {
	uint [];
};
layout (packed, std430, binding=) readonly buffer {
	uint [];
};
layout (packed, std430, binding=2) writeonly buffer bbb{
	uint outOffsets[];
};
layout (packed, std430, binding=3) writeonly buffer ccc {
	uint outCounts[];
};

uniform uint entitesCount;
uniform uint reduceCount;

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main() {
	const uint outId = gl_GlobalInvocationID.x;
	const uint startId = outId * reduceCount;
	const uint endId = min(startId + reduceCount, countIn);
	uint count = 0;
	for(uint id=startId; id<endId; ++id) {
		outOffsets[id] = count;
		count += inCounts[id];
	}
	if(startId < countIn) {
		outCounts[outId] = count;
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

layout (packed, std430, binding=5) writeonly buffer aaa {
	DrawElementsIndirectCommand indirectCommands[];
};
layout (packed, std430, binding=6) readonly buffer bbb {
	PerEntityMeshInfo meshInfo[];
};
layout (packed, std430, binding=7) readonly buffer ccc {
	uint visibleEntityIds[];
};

uniform uint entitiesCount;

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

