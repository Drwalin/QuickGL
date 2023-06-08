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
#include "../../OpenGLWrapper/include/openglwrapper/basic_mesh_loader/Mesh.hpp"

#include "../../include/quickgl/AnimatedMeshManager.hpp"
#include "../../include/quickgl/cameras/Camera.hpp"
#include "../../include/quickgl/Engine.hpp"

#include "../../include/quickgl/pipelines/PipelineAnimated.hpp"

namespace qgl {
	PipelineAnimated::PipelineAnimated() {
	}
	
	PipelineAnimated::~PipelineAnimated() {
	}
	
	std::string PipelineAnimated::GetPipelineName() const {
		return "PipelineAnimated";
	}
	
	uint32_t PipelineAnimated::CreateEntity() {
		uint32_t entity = PipelineFrustumCulling::CreateEntity();
		SetAnimationState(entity, 0, 0, false, 0, false);
		return entity;
	}
	
	void PipelineAnimated::SetAnimationState(uint32_t entityId,
			uint32_t animationId, float timeOffset, bool enableUpdateTime,
			uint32_t animationIdAfter, bool continueNextAnimation) {
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
	
	void PipelineAnimated::Initialize() {
		PipelineFrustumCulling::Initialize();
		
		perEntityAnimationState.Init();
		
		// init shaders
		renderShader = std::make_unique<gl::Shader>();
		if(renderShader->Compile(VERTEX_SHADER_SOURCE, "", FRAGMENT_SHADER_SOURCE))
			exit(31);
		
		updateAnimationShader = std::make_unique<gl::Shader>();
		if(updateAnimationShader->Compile(UPDATE_ANIMATION_SHADER_SOURCE))
			exit(31);
		
		// init vao
		vao = std::make_unique<gl::VAO>(gl::TRIANGLES);
		vao->Init();
		gl::VBO& vbo = meshManager->GetVBO();
		
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_pos"), 3, gl::FLOAT, false, 0, 0);
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_color"), 4, gl::UNSIGNED_BYTE, true, 12, 0);
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_normal"), 4, gl::BYTE, true, 16, 0);
		
		vao->SetAttribPointer(     vbo, renderShader->GetAttributeLocation("in_weight"), 4, gl::UNSIGNED_BYTE, true, 20, 0);
		vao->SetIntegerAttribPointer(vbo, renderShader->GetAttributeLocation("in_bones"), 4, gl::UNSIGNED_BYTE, 24, 0);
		
		// init animation state vertex attribute
		vao->SetIntegerAttribPointer(perEntityAnimationState.Vbo(),
				renderShader->GetAttributeLocation("in_animationState"),
				4, gl::UNSIGNED_INT, 12, 1);
		
		// init model matrix
		gl::VBO& modelVbo = transformMatrices.Vbo();
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+0, 4, gl::FLOAT, false, 0, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+1, 4, gl::FLOAT, false, 16, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+2, 4, gl::FLOAT, false, 32, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+3, 4, gl::FLOAT, false, 48, 1);
		vao->BindElementBuffer(meshManager->GetEBO(), gl::UNSIGNED_INT);
	}
	
	uint32_t PipelineAnimated::FlushDataToGPU(uint32_t stageId) {
		uint32_t ret = perEntityAnimationState.UpdateVBO(stageId);
		ret = std::max(ret, PipelineFrustumCulling::FlushDataToGPU(stageId));
		return ret;
	}
	
	void PipelineAnimated::GenerateRenderStages(std::vector<Stage>& stages) {
		PipelineFrustumCulling::GenerateRenderStages(stages);
		
		{
		const int32_t ENTITIES_COUNT_LOCATION =
			updateAnimationShader->GetUniformLocation("entitiesCount");
		const int32_t DELTA_TIME_LOCATION =
			updateAnimationShader->GetUniformLocation("deltaTime");
		const int32_t TIME_STAMP_LOCATION =
			updateAnimationShader->GetUniformLocation("timeStamp");
		
		int loc = -1;
		{
			for(int i=0; i<stages.size(); ++i) {
				if(stages[i].stageType != STAGE_GLOBAL) {
					loc = i;
					break;
				}
			}
			if(loc < 0)
				loc = stages.size();
		}
		stages.insert(stages.begin()+loc, std::move(Stage(
			"Update animation info",
			STAGE_GLOBAL,
			[=](std::shared_ptr<Camera> camera) {
				updateAnimationShader->Use();
				glMemoryBarrier(GL_ALL_BARRIER_BITS);
				
				updateAnimationShader->SetUInt(ENTITIES_COUNT_LOCATION,
						GetEntitiesCount());
				updateAnimationShader->SetFloat(DELTA_TIME_LOCATION,
						engine->GetInputManager().GetDeltaTime());
				updateAnimationShader->SetFloat(TIME_STAMP_LOCATION,
						engine->GetInputManager().GetTime());
				
				animatedMeshManager->GetAnimationManager()
					.GetAnimationsMetadata().Vbo()
					.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 2);
				
				perEntityAnimationState.Vbo()
					.BindBufferBase(gl::SHADER_STORAGE_BUFFER, 1);
				
				updateAnimationShader->DispatchRoundGroupNumbers(
						GetEntitiesCount(), 1, 1);
			})));
		}
		
		{
		// get shader uniform locations
		const int32_t PROJECTION_VIEW_LOCATION =
			renderShader->GetUniformLocation("projectionView");
		
		stages.emplace_back(
			"Render bone animated entities",
			STAGE_PER_CAMERA_FBO,
			[=](std::shared_ptr<Camera> camera) {
				// draw with indirect draw buffer
				renderShader->Use();
				
				glm::mat4 pv = camera->GetPerspectiveMatrix()
					* camera->GetViewMatrix();
				renderShader->SetMat4(PROJECTION_VIEW_LOCATION, pv);
				vao->BindIndirectBuffer(*indirectDrawBuffer);
				
				vao->DrawMultiElementsIndirect(NULL,
						frustumCulledEntitiesCount);
			});
		}
	}
	
	std::shared_ptr<MeshManager> PipelineAnimated::CreateMeshManager() {
		static constexpr uint32_t stride
			= 3*sizeof(float)   // pos
			+ 4*sizeof(uint8_t) // color
			+ 4*sizeof(uint8_t) // normal
			+ 8*sizeof(uint8_t) // bones and weights
			;
		
		animatedMeshManager = std::make_shared<AnimatedMeshManager>(stride,
			[](std::vector<uint8_t>& buffer, uint32_t offset,
					gl::BasicMeshLoader::Mesh* mesh){
				
				mesh->ExtractPos<float>(offset, buffer, 0, stride,
						gl::BasicMeshLoader::ConverterFloatPlain<float, 3>);
				
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
			});
		return animatedMeshManager;
	}
	
	const char* PipelineAnimated::VERTEX_SHADER_SOURCE = R"(
#version 450 core

in vec3 in_pos;
in vec4 in_color;
in vec3 in_normal;
in uvec4 in_bones;
in vec4 in_weight;

in uvec4 in_animationState; // {firstAnimationMatrixId, secondAnimationMatrixId,
                            // interpolactionFactor}
in mat4 model;

uniform mat4 projectionView;
uniform sampler2DArray bones;

out vec4 color;
out vec3 normal;
out vec4 pos;

const uint BONE_FRAMES_W = 64;
const uint BONE_FRAMES_H = 16384;

mat4 GetBonePose(uint frameStart, uint bone);
mat4 GetFrameMatrix(uint frameStart);
mat4 GetPoseBoneMatrix();

void main() {
	mat4 poseMat = GetPoseBoneMatrix();
	pos = model * poseMat * vec4(in_pos, 1);
	gl_Position = projectionView * pos;
	normal = normalize((model * poseMat * vec4(in_normal, 0)).xyz);
	color = in_color;
}

mat4 GetPoseBoneMatrix() {
	mat4 poseA = GetFrameMatrix(in_animationState.x); 
	mat4 poseB = GetFrameMatrix(in_animationState.y); 
	float factorA = uintBitsToFloat(in_animationState.z);
	float factorB = 1.0 - factorA;
	return (poseA * factorB) + (poseB * factorA);
}

mat4 GetFrameMatrix(uint frameStart) {
	return
		(GetBonePose(frameStart, in_bones[0])) * in_weight[0]
		+ (GetBonePose(frameStart, in_bones[1]) * in_weight[1])
		+ (GetBonePose(frameStart, in_bones[2]) * in_weight[2])
		+ (GetBonePose(frameStart, in_bones[3]) * in_weight[3])
	;
}

mat4 GetBonePose(uint frameStart, uint bone) {
	uint id = (frameStart+bone)*4;
	ivec3 p;
	p.x = int(id % BONE_FRAMES_W);
	id = id / BONE_FRAMES_W;
	p.y = int(id % BONE_FRAMES_H);
	id = id / BONE_FRAMES_H;
	p.z = int(id);
	return mat4(texelFetch(bones, p+ivec3(0,0,0), 0),
				texelFetch(bones, p+ivec3(1,0,0), 0),
				texelFetch(bones, p+ivec3(2,0,0), 0),
				texelFetch(bones, p+ivec3(3,0,0), 0));
}
)";
	
	const char* PipelineAnimated::FRAGMENT_SHADER_SOURCE = R"(
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
	
	const char* PipelineAnimated::UPDATE_ANIMATION_SHADER_SOURCE = R"(
#version 450 core

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

layout (packed, std430, binding=1) buffer aaa {
	AnimatedState animatedState[];
};

layout (packed, std430, binding=2) readonly buffer bbb {
	AnimationMetadata animationMetadata[];
};

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

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

