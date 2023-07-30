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
#include "../../OpenGLWrapper/include/openglwrapper/Sync.hpp"

#include "../../include/quickgl/MeshManager.hpp"
#include "../../include/quickgl/Engine.hpp"
#include "../../include/quickgl/IndirectDrawBufferGenerator.hpp"
#include "../../include/quickgl/cameras/Camera.hpp"
#include "../../include/quickgl/util/RenderStageComposer.hpp"

#include "../../include/quickgl/pipelines/PipelineFrustumCuling.hpp"

namespace qgl {
	PipelineFrustumCulling::PipelineFrustumCulling(std::shared_ptr<Engine> engine) :
		PipelineIdsManagedBase(engine) {
	}
	
	PipelineFrustumCulling::~PipelineFrustumCulling() {
	}
	
	uint32_t PipelineFrustumCulling::GetEntitiesToRender() const {
		return frustumCulledEntitiesCount;
	}
	
	void PipelineFrustumCulling::Init() {
		PipelineIdsManagedBase::Init();
		
		// init buffer objects
		frustumCulledIdsBuffer = std::make_shared<gl::VBO>(sizeof(uint32_t),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		frustumCulledIdsBuffer->Init();
		
		clippingPlanes = std::make_shared<gl::VBO>(sizeof(float),
				gl::SHADER_STORAGE_BUFFER, gl::DYNAMIC_DRAW);
		clippingPlanes->Init(128);
		
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
		
		frustumCullingShader = std::make_unique<gl::Shader>();
		if(frustumCullingShader->Compile(FRUSTUM_CULLING_COMPUTE_SHADER_SOURCE))
			exit(31);
		UNIFORM_LOCATION_DEPTH_TEXTURE = frustumCullingShader->GetUniformLocation("depthTexture");
		
		objectsPerInvocation = 16;
		
		indirectDrawBuffer = std::make_shared<gl::VBO>(20,
				gl::DRAW_INDIRECT_BUFFER, gl::DYNAMIC_DRAW);
		indirectDrawBuffer->Init(1024);
		
		stagesScheduler.AddStage(
			"Update frustum culling data",
			STAGE_UPDATE_DATA,
			&PipelineFrustumCulling::UpdateFrustumCullingData);
		
		stagesScheduler.AddStage(
			"Updating clipping planes of camera to GPU",
			STAGE_CAMERA,
			&PipelineFrustumCulling::UpdateClippingPlanesOfCameraToGPU);
	
		stagesScheduler.AddStage(
			"Performing frustum culling",
			STAGE_CAMERA,
			&PipelineFrustumCulling::PerformFrustumCulling);

		stagesScheduler.AddStage(
			"Fetching count of entities in frustum view to CPU",
			STAGE_CAMERA,
			&PipelineFrustumCulling::FetchFrustumCulledEntitiesCount,
			&PipelineFrustumCulling::CanExecuteFetchFrustumCulledEntitiesCount);
		
		stagesScheduler.AddStage(
			"Generating indirect draw command buffer",
			STAGE_CAMERA,
			&PipelineFrustumCulling::GenerateIndirectDrawCommandBuffer);
	}
	
	void PipelineFrustumCulling::UpdateFrustumCullingData(std::shared_ptr<Camera> camera) {
		uint32_t i = frustumCulledIdsBuffer->GetVertexCount();
		while(i < entityBufferManager->Count()) {
			i = (i*3)/2 + 100;
		}
		if(i != frustumCulledIdsBuffer->GetVertexCount()) {
			frustumCulledIdsBuffer->Generate(nullptr, i);
		}
	}
		
	void PipelineFrustumCulling::UpdateClippingPlanesOfCameraToGPU(std::shared_ptr<Camera> camera) {
		struct {
			glm::mat4 pv;
			glm::mat4 prevPV;
			glm::mat4 cameraInverseTransform;
			glm::vec4 up;
			glm::vec4 right;
			glm::vec4 front;
			glm::vec4 nearfar;
			glm::vec4 clippingPlanes[5];
			glm::vec4 p1fur;
			glm::vec4 p2fur;
			glm::vec4 p3fur;
			glm::uvec2 cameraPixelDimension;
			uint objectsPerInvocation;
			uint entitiesCount;
		} d;
		camera->GetClippingPlanes(d.clippingPlanes);
		d.pv = camera->GetPerspectiveViewMatrix();
		d.prevPV = camera->GetPreviousPerspectiveViewMatrix();
		d.cameraInverseTransform = camera->GetViewMatrix();
		d.up = glm::vec4(camera->GetUp(), 0);
		d.right = glm::vec4(camera->GetRight(), 0);
		d.front = glm::vec4(camera->GetFront(), 0);
		d.p1fur = d.front - d.up - d.right;
		d.p2fur = d.front + d.up + d.right;
		d.p3fur = - d.front;
		d.nearfar = {camera->GetNear(), camera->GetFar(), 0, 0};
		camera->GetRenderTargetDimensions(d.cameraPixelDimension.x, d.cameraPixelDimension.y);
		d.objectsPerInvocation = objectsPerInvocation;
		d.entitiesCount = entityBufferManager->Count();
		clippingPlanes->Update(&d, 0, sizeof(d));
		
		frustumCulledEntitiesCount = 0;
		frustumCulledIdsCountAtomicCounter
			->Update(&frustumCulledEntitiesCount, 0, sizeof(uint32_t));
	}
	
	void PipelineFrustumCulling::PerformFrustumCulling(std::shared_ptr<Camera> camera) {
		// set visible entities count
		frustumCullingShader->Use();

		// bind buffers
		frustumCulledIdsBuffer
			->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
		transformMatrices.Vbo()
			.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 3);
		frustumCulledIdsCountAtomicCounter
			->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 4);
		clippingPlanes
			->BindBufferBase(gl::SHADER_STORAGE_BUFFER, 5);
		perEntityMeshInfoBoundingSphere.Vbo()
			.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 6);
		frustumCullingShader
			->SetTexture(UNIFORM_LOCATION_DEPTH_TEXTURE,
				camera->GetDepthTexture().get(), 0);

		// perform frustum culling
		
		frustumCullingShader
			->DispatchRoundGroupNumbers(
					(entityBufferManager->Count()+objectsPerInvocation-1) /
						objectsPerInvocation,
					1, 1);
		gl::Shader::Unuse();
		
		frustumCulledIdsCountAtomicCounterAsyncFetch->
			Copy(frustumCulledIdsCountAtomicCounter.get(), 0, 0, 12);
		
		frustumCulledIdsCountAtomicCounterAsyncFetch->
			FlushFromGpuMapPersistentFullRange();
		
		syncFrustumCulledEntitiesCountReadyToFetch.StartFence();
	}

	void PipelineFrustumCulling::FetchFrustumCulledEntitiesCount(std::shared_ptr<Camera> camera) {
		// wait for fence
		if(syncFrustumCulledEntitiesCountReadyToFetch.WaitClient(100*1000*1000) == gl::SYNC_TIMEOUT) {
			gl::Finish();
		}
		syncFrustumCulledEntitiesCountReadyToFetch.Destroy();

		// fetch number of entities to render after culling
		frustumCulledEntitiesCount = mappedPointerToentitiesCount[0];

		if(indirectDrawBuffer->GetVertexCount() < frustumCulledEntitiesCount) {
			indirectDrawBuffer->Generate(nullptr,
					(frustumCulledEntitiesCount | 0xFFF) + 1);
		}
		
// 		gl::MemoryBarrier(gl::ALL_BARRIER_BITS);
	}

	bool PipelineFrustumCulling::CanExecuteFetchFrustumCulledEntitiesCount(std::shared_ptr<Camera> camera) {
		return syncFrustumCulledEntitiesCountReadyToFetch.IsDone();
	}
		
	void PipelineFrustumCulling::GenerateIndirectDrawCommandBuffer(std::shared_ptr<Camera> camera) {
		engine->GetIndirectDrawBufferGenerator()->Generate(
				*frustumCulledIdsBuffer,
				perEntityMeshInfo.Vbo(),
				*indirectDrawBuffer,
				frustumCulledEntitiesCount,
				0);

		gl::MemoryBarrier(gl::BUFFER_UPDATE_BARRIER_BIT |
				gl::SHADER_STORAGE_BARRIER_BIT |
				gl::UNIFORM_BARRIER_BIT | gl::COMMAND_BARRIER_BIT);
	}
	
	
	
	
	void PipelineFrustumCulling::Destroy() {
		frustumCullingShader->Destroy();
		frustumCulledIdsBuffer->Destroy();
		frustumCulledIdsCountAtomicCounter->Destroy();
		frustumCulledIdsCountAtomicCounterAsyncFetch->Destroy();
		clippingPlanes->Destroy();
		
		frustumCullingShader = nullptr;
		frustumCulledIdsBuffer = nullptr;
		frustumCulledIdsCountAtomicCounter = nullptr;
		frustumCulledIdsCountAtomicCounterAsyncFetch = nullptr;
		clippingPlanes = nullptr;
		
		syncFrustumCulledEntitiesCountReadyToFetch.Destroy();
		
		mappedPointerToentitiesCount = nullptr;
		
		PipelineIdsManagedBase::Destroy();
	}
	
	const char* PipelineFrustumCulling::FRUSTUM_CULLING_COMPUTE_SHADER_SOURCE = R"(
#version 420 core
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_storage_buffer_object : require

layout (std430, binding=1) writeonly buffer aaa {
	uint frustumCulledEntitiesIds[];
};
layout (std430, binding=3) readonly buffer ccc {
	mat4 entitesTransformations[];
};
layout (std430, binding=4) buffer ddd {
	uint globalAtomicCounter;
};
layout (std430, binding=5) readonly buffer eee {
	mat4 pv;
	mat4 prevPV;
	mat4 cameraInverseTransform;
	vec4 up;
	vec4 right;
	vec4 front;
	vec4 nearfar;
	vec4 clippingPlanes[5];
	vec4 p1fur;
	vec4 p2fur;
	vec4 p3fur;
	ivec2 cameraPixelDimension;
	uint objectsPerInvocation;
	uint entitiesCount;
};
layout (std430, binding=6) readonly buffer fff {
	vec4 meshInfo[];
};

layout (local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

shared uint localAtomicCounter;
shared uint commonStartingLocation;

uniform sampler2D depthTexture;

uint IsFullyInViewport(uint id, mat4 matPV, out vec4 _p1, out vec4 _p2, out vec4 _p3) {
	vec4 pos = entitesTransformations[id] * vec4(meshInfo[id].xyz, 1);
	vec4 rad = entitesTransformations[id] * vec4(0,0,meshInfo[id].w, 0);
	float dd = length(rad);
	
	_p1 = matPV*(pos + p1fur * dd);
	_p2 = matPV*(pos + p2fur * dd);
	_p3 = matPV*(pos + p3fur * dd);
	
	_p1.xyz /= _p1.w;
	_p2.xyz /= _p2.w;
	_p3.xyz /= _p3.w;
	
	vec4 p1 = _p1;
	vec4 p2 = _p2;
	vec4 p3 = _p3;
	
	
// 	if(p1.w >= 0 && p3.w <= nearfar.y && p1.x >= -1 && p1.y >= -1 && p2.x <= 1 && p2.y <= 1)
// 		return 1;
// 	else
// 		return 0;


// 	p1.w >= 0
// 	p3.w <= nearfar.y
// 	p1.x >= -1
// 	p1.y >= -1
// 	p2.x <= -1
// 	p2.y <= -1

	p1.w = p1.w - 0;
	p3.w = -(p3.w - nearfar.y);
	p1.x = p1.x - (-1);
	p1.y = p1.y - (-1);
	p2.x = -(p2.x - 1);
	p2.y = -(p2.y - 1);
	
	uvec4 u1, u2, u3;
	u1.w = floatBitsToUint(p1.w);
	u3.w = floatBitsToUint(p3.w);
	u1.x = floatBitsToUint(p1.x);
	u1.y = floatBitsToUint(p1.y);
	u2.x = floatBitsToUint(p2.x);
	u2.y = floatBitsToUint(p2.y);
	
	return (((u1.w | u3.w | u1.x | u1.y | u2.x | u2.y)>>31) & 1) ^ 1;
}

uint IsInViewport(uint id, mat4 matPV, out vec4 _p1, out vec4 _p2, out vec4 _p3) {
	vec4 pos = entitesTransformations[id] * vec4(meshInfo[id].xyz, 1);
	vec4 rad = entitesTransformations[id] * vec4(0,0,meshInfo[id].w, 0);
	float dd = length(rad);
	
	_p1 = matPV*(pos + p1fur * dd);
	_p2 = matPV*(pos + p2fur * dd);
	_p3 = matPV*(pos + p3fur * dd);
	
	_p1.xyz /= _p1.w;
	_p2.xyz /= _p2.w;
	_p3.xyz /= _p3.w;
	
	vec4 p1 = _p1;
	vec4 p2 = _p2;
	vec4 p3 = _p3;

	
	if(p1.w >= 0 && p3.w <= nearfar.y && p1.x <= 1 && p1.y <= 1 && p2.x >= -1 && p2.y >= -1)
		return 1;
	else
		return 0;
	
	
// 	p1.w >= 0
// 	p3.w <= nearfar.y
// 	p1.x <= 1
// 	p1.y <= 1
// 	p2.x >= -1
// 	p2.y >= -1

	p1.w = p1.w - 0;
	p3.w = -(p3.w - nearfar.y);
	p1.x = -(p1.x - 1);
	p1.y = -(p1.y - 1);
	p2.x = p2.x - (-1);
	p2.y = p2.y - (-1);
	
	uvec4 u1, u2, u3;
	u1.w = floatBitsToUint(p1.w);
	u3.w = floatBitsToUint(p3.w);
	u1.x = floatBitsToUint(p1.x);
	u1.y = floatBitsToUint(p1.y);
	u2.x = floatBitsToUint(p2.x);
	u2.y = floatBitsToUint(p2.y);
	
	return (((u1.w | u3.w | u1.x | u1.y | u2.x | u2.y)>>31) & 1) ^ 1;
}

uint IsNotOccluded(vec4 p1, vec4 p2, vec4 p3) {
	if(p3.z*p3.w < 1) {
		return 1;
	}
	
	ivec2 ss1 = ivec2(clamp(p1.xy*0.5 + 0.5, vec2(0,0), vec2(1,1)) * (cameraPixelDimension-1));
	ivec2 ss2 = ivec2(clamp(p2.xy*0.5 + 0.5, vec2(0,0), vec2(1,1)) * (cameraPixelDimension));
	const ivec2 s1 = max(ss1, ivec2(0,0));//min(ss1, ss2);
	const ivec2 s2 = ss2;//max(ss1, ss2);
	const float currentDepth = (p3.z * 0.5) + 0.5 - 0.00007;

	const ivec2 s = abs(s2-s1);
	const int smaxdim = min(cameraPixelDimension.x, cameraPixelDimension.y);
	const int maxlod = int(log2(smaxdim))-2;
	
	const int omaxdim = max(s.x, s.y);
	const int lod = max(min(int(log2(omaxdim)), maxlod), 2);
	const int bits = (1<<lod) - 1;
	
	ivec2 end = min((s2+bits)>>lod, ((cameraPixelDimension+bits)>>lod));
	ivec2 start = max(min((s1)>>lod, end), ivec2(0,0));
	end = max(end, start);
	
	for(int i=start.x; i<=end.x; ++i) {
		for(int j=start.y; j<=end.y; ++j) {
			float testedDepth = texelFetch(depthTexture, ivec2(i,j), lod-1).x;
			if(currentDepth <= testedDepth) {
				return 1;
			}
		}
	}
	return 0;
}

uint IsInViewNotOccluded(uint id) {
	if(id >= entitiesCount) {
		return 0;
	}
	
	vec4 p1, p2, p3;
	if(IsInViewport(id, pv, p1, p2, p3) == 0) {
		return 0;
	}
	
	if(IsInViewport(id, prevPV, p1, p2, p3) == 1) {
		return IsNotOccluded(p1, p2, p3);
	}
	
	return 1;
	return 0;
}

const uint MAX_OBJECTS_PER_INVOCATION = 16;

void main() {
	if(gl_LocalInvocationID.x == 0)
		localAtomicCounter = 0;
	barrier();
	
	uint inViewCount = 0;
	uint inViewIds[MAX_OBJECTS_PER_INVOCATION];

	for(uint i=0; i<objectsPerInvocation; ++i) {
		uint invocationId = gl_GlobalInvocationID.x*objectsPerInvocation + i;
		uint isIn = IsInViewNotOccluded(invocationId);
		inViewIds[inViewCount] = invocationId;
		inViewCount += isIn;
	}
	
	uint localStartingLocation = 0;
	if(inViewCount > 0) // @TODO: this condition can be removed
												// and code still will work:
												// @TODO: check if removign this condition is faster
		localStartingLocation = atomicAdd(localAtomicCounter, inViewCount);
	
	barrier();
	if(gl_LocalInvocationID.x == 0)
		commonStartingLocation = atomicAdd(globalAtomicCounter, localAtomicCounter);
	barrier();
	
	uint globalStartingLocation = commonStartingLocation+localStartingLocation;
	
	for(uint i=0; i<inViewCount; ++i) {
		frustumCulledEntitiesIds[globalStartingLocation+i] = inViewIds[i];
	}
}
)";
}

