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
#include <algorithm>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VAO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Shader.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/basic_mesh_loader/Mesh.hpp"

#include "../../include/quickgl/AnimatedMeshManager.hpp"
#include "../../include/quickgl/cameras/Camera.hpp"
#include "../../include/quickgl/Engine.hpp"
#include "../../include/quickgl/materials/MaterialBoneAnimated.hpp"

#include "../../include/quickgl/pipelines/PipelineBoneAnimated.hpp"

namespace qgl {
	PipelineBoneAnimated::PipelineBoneAnimated(std::shared_ptr<Engine> engine) :
		PipelineFrustumCulling(engine), perEntityAnimationState(engine) {
	}
	
	PipelineBoneAnimated::~PipelineBoneAnimated() {
	}
	
	std::string PipelineBoneAnimated::GetName() const {
		return "PipelineBoneAnimated";
	}
	
	uint32_t PipelineBoneAnimated::CreateEntity() {
		uint32_t entity = PipelineFrustumCulling::CreateEntity();
		SetAnimationState(entity, 0, 0, false, 0, false);
		return entity;
	}
	
	void PipelineBoneAnimated::SetAnimationState(uint32_t entityId,
			uint32_t animationId, float timeOffset, bool enableUpdateTime,
			uint32_t animationIdAfter, bool continueNextAnimation) {
		entityId = GetEntityOffset(entityId);
		perEntityAnimationState.SetValue({
				animationId,
				animationIdAfter,
				(continueNextAnimation?1u:0u) | (enableUpdateTime?2u:0u),
				0,
				0,
				0,
				timeOffset,
				engine->GetInputManager().GetTime()
			}, entityId);
	}
	
	void PipelineBoneAnimated::Init() {
		material = std::make_shared<MaterialBoneAnimated>(
				std::dynamic_pointer_cast<PipelineBoneAnimated>(
					shared_from_this()));
		
		PipelineFrustumCulling::Init();
		
		perEntityAnimationState.Init();
		
		material->Init();
		
		updateAnimationShader = std::make_unique<gl::Shader>();
		if(updateAnimationShader->Compile(UPDATE_ANIMATION_SHADER_SOURCE))
			exit(31);
		
		entityBufferManager
			->AddManagedSparselyUpdateVBO(&perEntityAnimationState);
		
		ENTITIES_COUNT_LOCATION =
			updateAnimationShader->GetUniformLocation("entitiesCount");
		DELTA_TIME_LOCATION =
			updateAnimationShader->GetUniformLocation("deltaTime");
		TIME_STAMP_LOCATION =
			updateAnimationShader->GetUniformLocation("timeStamp");
		
		stagesScheduler.AddStage(
			"Update animation data",
			STAGE_UPDATE_DATA,
			&PipelineBoneAnimated::UpdateAnimationData);
		
		stagesScheduler.AddStage(
			"Update animation state",
			STAGE_GLOBAL,
			&PipelineBoneAnimated::UpdateAnimationState);
		
		stagesScheduler.AddStage(
			"Render bone animated entities",
			STAGE_1_RENDER_PASS_1,
			&PipelineBoneAnimated::RenderEntities);
	}
	
	void PipelineBoneAnimated::UpdateAnimationData(std::shared_ptr<Camera> camera) {
		perEntityAnimationState.UpdateVBO();
	}
	
	void PipelineBoneAnimated::UpdateAnimationState(std::shared_ptr<Camera> camera) {
		updateAnimationShader->Use();

		updateAnimationShader->SetUInt(ENTITIES_COUNT_LOCATION,
				GetEntitiesCount());
		updateAnimationShader->SetFloat(DELTA_TIME_LOCATION,
				engine->GetInputManager().GetDeltaTime());
		updateAnimationShader->SetFloat(TIME_STAMP_LOCATION,
				engine->GetInputManager().GetTime());

		perEntityAnimationState.Vbo()
			.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
		animatedMeshManager->GetAnimationManager()
			.GetAnimationsMetadata().Vbo()
			.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 2);

		updateAnimationShader->DispatchRoundGroupNumbers(
				GetEntitiesCount(), 1, 1);
		gl::Shader::Unuse();
	}
	
	void PipelineBoneAnimated::RenderEntities(std::shared_ptr<Camera> camera) {
		material->RenderPassIndirect(camera, *indirectDrawBuffer,
				frustumCulledEntitiesCount);
	}
	
	void PipelineBoneAnimated::Destroy() {
		perEntityAnimationState.Destroy();
		
		updateAnimationShader->Destroy();
		updateAnimationShader = nullptr;
		
		animatedMeshManager = nullptr;
	}
	
	std::shared_ptr<MeshManager> PipelineBoneAnimated::CreateMeshManager() {
		static constexpr uint32_t stride
			= 3*sizeof(float)   // pos
			+ 4*sizeof(uint8_t) // color
			+ 4*sizeof(uint8_t) // normal
			+ 8*sizeof(uint8_t) // bones and weights
			;
		
		animatedMeshManager = std::make_shared<AnimatedMeshManager>(stride,
			[](std::vector<uint8_t>& buffer, uint32_t offset,
					gl::BasicMeshLoader::Mesh* mesh)->bool {
			
				if(mesh->weight.size() == 0) {
					return false;
				}
				
				mesh->ExtractPos<float>(offset, buffer, 0, stride,
						gl::BasicMeshLoader::ConverterFloatPlain<float, 3>);
				
				if(mesh->color.size() == 0 || mesh->color[0].size() != mesh->pos.size()) {
					mesh->color.resize(std::max<int>(mesh->color.size(), 1));
					mesh->color[0].resize(mesh->pos.size());
					for(auto&c : mesh->color[0]) {
						c = {0,0,0,1};
					}
				}
				mesh->ExtractColor<uint8_t>(offset, buffer, 12, stride,
						gl::BasicMeshLoader::ConverterIntPlainClampScale
							<uint8_t, 255, 0, 255, 4>);
				
				mesh->ExtractNormal(offset, buffer, 16, stride,
						gl::BasicMeshLoader::ConverterIntNormalized
							<uint8_t, 127, 3>);
				
				mesh->ExtractWeightsWithBones<uint8_t, uint8_t>(offset, buffer,
						20, 24, stride,
						gl::BasicMeshLoader::ConverterIntPlainClampScale
							<uint8_t, 255, 0, 255, 1>, 4);
				
				return true;
			});
		return animatedMeshManager;
	}
	
	const char* PipelineBoneAnimated::UPDATE_ANIMATION_SHADER_SOURCE = R"(
#version 420 core
#extension GL_ARB_compute_shader : require
#extension GL_ARB_shader_storage_buffer_object : require

struct AnimatedState {
	uint animationId;
	uint animationIdAfter;
	uint flags; // 1 - continueNextAnimation, 2 - updateTime
	
	uint firstMatrixFrameCurrent;
	uint firstMatrixFrameNext;
	float interpolationFactor;
	
	float timeOffset;
	float lastAccessTimeStamp;
};

struct AnimationMetadata {
	uint firstMatrixId;
	uint bonesCount;
	uint framesCount;
	uint fps;
};

uniform float deltaTime;
uniform float timeStamp;
uniform uint entitiesCount;

layout (std430, binding=1) buffer aaa {
	AnimatedState animatedState[];
};

layout (std430, binding=2) readonly buffer bbb {
	AnimationMetadata animationMetadata[];
};

layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

void main() {
	uint id = gl_GlobalInvocationID.x;
	if(id >= entitiesCount)
		return;
	AnimatedState s = animatedState[id];

	if((s.flags & 2) == 2) {
		s.timeOffset += deltaTime;
	}
	
	s.lastAccessTimeStamp = timeStamp;
	AnimationMetadata a = animationMetadata[s.animationId];
	uint currentFrame, nextFrame;
	float frameOffset = s.timeOffset * a.fps;
	currentFrame = int(floor(frameOffset));
	nextFrame = currentFrame + 1;
	
	if(nextFrame < a.framesCount) { // continue current animation
		s.firstMatrixFrameCurrent =
				currentFrame * a.bonesCount + a.firstMatrixId;
		s.firstMatrixFrameNext = s.firstMatrixFrameCurrent + a.bonesCount;
		s.interpolationFactor = fract(frameOffset);
	} else if((s.flags & 1) == 1) { // continue next animation
		AnimationMetadata a2 = animationMetadata[s.animationIdAfter];
		if(nextFrame > a.framesCount) { // next animation is already in play
			s.animationId = s.animationIdAfter;
			frameOffset -= a.framesCount;
			currentFrame -= a.framesCount;
			uint c = ((currentFrame/a2.framesCount)*a2.framesCount);
			currentFrame -= c;
			frameOffset -= c;
			nextFrame = (currentFrame + 1) % a2.framesCount;
			s.timeOffset = frameOffset/a2.fps;
			s.firstMatrixFrameCurrent = currentFrame * a2.bonesCount + a2.firstMatrixId;
			s.firstMatrixFrameNext = nextFrame * a2.bonesCount + a2.firstMatrixId;
		} else { // transitioning animations
			s.firstMatrixFrameCurrent = (a.framesCount-1) * a.bonesCount + a.firstMatrixId;
			s.firstMatrixFrameNext = a2.firstMatrixId;
		}
		s.interpolationFactor = fract(frameOffset);
	} else { // stop at last frame of animation
		s.firstMatrixFrameCurrent =
				(a.framesCount-1) * a.bonesCount + a.firstMatrixId;
		s.firstMatrixFrameNext = s.firstMatrixFrameCurrent;
		s.interpolationFactor = 0;
	}
	
	animatedState[id] = s;
}
)";
}

