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

#include "../../include/quickgl/AnimatedMeshManager.hpp"
#include "../../include/quickgl/cameras/Camera.hpp"
#include "../../include/quickgl/Engine.hpp"

#include "../../include/quickgl/pipelines/PipelineAnimated.hpp"

namespace qgl {
	PipelineAnimated::PipelineAnimated() {
	}
	
	PipelineAnimated::~PipelineAnimated() {
	}
	
	void PipelineAnimated::SetAnimationState(uint32_t entityId,
			uint32_t animationId, float timeOffset, bool enableUpdateTime,
			uint32_t animationIdAfter, bool loop) {
		perEntityAnimationState.SetValue({
				animationId,
				animationIdAfter,
				(loop?1u:0u) << 0 | (enableUpdateTime?1u:0u) << 1,
				
				0,
				0,
				0,
				
				engine->GetInputManager().GetTime(),
				engine->GetInputManager().GetTime()
			}, entityId);
	}
	
	void PipelineAnimated::Initialize() {
		PipelineFrustumCulling::Initialize();
		
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
		
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_weights"), 4, gl::UNSIGNED_BYTE, true, 24, 0);
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_bones"), 4, gl::UNSIGNED_BYTE, false, 20, 0);
		
		// init animation state vertex attribute
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_animationState"), 4, gl::UNSIGNED_INT, false, 16, 1);
		
		// init model matrix
		gl::VBO& modelVbo = transformMatrices.Vbo();
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+0, 4, gl::FLOAT, false, 0, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+1, 4, gl::FLOAT, false, 16, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+2, 4, gl::FLOAT, false, 32, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+3, 4, gl::FLOAT, false, 48, 1);
		vao->BindElementBuffer(meshManager->GetEBO(), gl::UNSIGNED_INT);
	}
	
	uint32_t PipelineAnimated::FlushDataToGPU(uint32_t stageId) {
		return perEntityAnimationState.UpdateVBO(stageId);
	}
	
	void PipelineAnimated::AppendRenderStages(std::vector<StageFunction>& stages) {
		PipelineFrustumCulling::AppendRenderStages(stages);
		
		{
		const int32_t ENTITIES_COUNT_LOCATION =
			updateAnimationShader->GetUniformLocation("entitiesCount");
		const int32_t DELTA_TIME_LOCATION =
			updateAnimationShader->GetUniformLocation("deltaTime");
		const int32_t TIME_STAMP_LOCATION =
			updateAnimationShader->GetUniformLocation("deltaTime");
		const int32_t ANIMATION_INFO_LOCATION =
			updateAnimationShader->GetUniformLocation("animationInfo");
		
		stages.emplace_back([=](std::shared_ptr<Camera> camera){
				updateAnimationShader->Use();
				glMemoryBarrier(GL_ALL_BARRIER_BITS);
				
				updateAnimationShader->SetUInt(ENTITIES_COUNT_LOCATION,
						GetEntitiesCount());
				updateAnimationShader->SetUInt(DELTA_TIME_LOCATION,
						engine->GetInputManager().GetDeltaTime());
				updateAnimationShader->SetUInt(TIME_STAMP_LOCATION,
						engine->GetInputManager().GetTime());
				updateAnimationShader->SetTexture(ANIMATION_INFO_LOCATION,
						animatedMeshManager->metaInfo.get(), 0);
				
				updateAnimationShader->DispatchRoundGroupNumbers(
						GetEntitiesCount(), 1, 1);
			});
		}
		
		{
		// get shader uniform locations
		const int32_t PROJECTION_VIEW_LOCATION =
			renderShader->GetUniformLocation("projectionView");
		
		stages.emplace_back([=](std::shared_ptr<Camera> camera){
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
						24, 20, stride,
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

in uvec4 in_animationState;
in mat4 model;

uniform mat4 projectionView;
uniform sampler2DArray bones;

out vec4 color;
out vec3 normal;
out vec4 pos;

void main() {
	gl_Position = pos = projectionView * model * vec4(in_pos, 1);
	normal = normalize((model * vec4(in_normal, 0)).xyz);
	color = in_color;
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

struct AnimationInfo {
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

layout (binding = 0, rgba32ui) readonly uniform image2D animationInfo;

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

AnimationInfo GetAnimationInfo(uint animId) {
	uvec4 v = imageLoad(animationInfo, animId);
	return {v.x, v.y, v.z, v.w};
}

void main() {
	uint id = gl_GlobalInvocationID.x;
	if(id >= entitiesCount)
		return;
	AnimatedState s = animatedState[id];

	if(s.flags & 2) {
		s.timeOffset += deltaTime;
	}
	
	s.lastAccessTimeStamp = timeStamp;

	AnimationInfo a = GetAnimationInfo(s.animationId);
	AnimationInfo a2 = GetAnimationInfo(s.animationIdAfter);

	float frame = s.timeOffset * (float)a.firstMatrixFramesCount;
	if(s.flags & 1) { // continue next animation
		frame = mod(frame, (float)a.framesCount);
		float nextFrame = mod(frame+1.0, (float)a.framesCount);
		s.firstMatrixFrameCurrent = floor(frame)*a.bonesCount + a.firstMatrixId;
		s.firstMatrixFrameNext = floor(nextFrame)*a.bonesCount + a.firstMatrixId;
		s.timeOffset = frame / (float)a.fps;
	} else { // stop at last frame of current animation
	}

// 	if(frame >= a.firstMatrixFramesCount+1) {
// 		if(s.flags & 1) {
// 			s.timeOffset -= (float)a.framesCount / (float)a.fps;
// 			s.firstMatrixFrameCurrent = a.framesCount*a.bonesCount
// 				+ a.firstMatrixId;
// 			s.firstMatrixFrameNext = = a.framesCount*a.bonesCount
// 				+ a.firstMatrixId;
// 		} else {
// 			s.firstMatrixFrameCurrent = a.framesCount*a.bonesCount
// 				+ a.firstMatrixId;
// 			s.firstMatrixFrameNext = = a.framesCount*a.bonesCount
// 				+ a.firstMatrixId;
// 		}
// 	} else if(frame == a.firstMatrixFramesCount) {
// 		s.firstMatrixFrameCurrent = floor(frame)*a.bonesCount + a.firstMatrixId;
// 		s.firstMatrixFrameNext = a2.firstMatrixId;
// 	} else {
// 		s.firstMatrixFrameCurrent = floor(frame)*a.bonesCount + a.firstMatrixId;
// 		s.firstMatrixFrameNext = s.firstMatrixFrameCurrent + a.bonesCount;
// 	}
	
	s.interpolationFactor = fract(frame);

	
	

	animatedState[id] = s;
}
)";
}

