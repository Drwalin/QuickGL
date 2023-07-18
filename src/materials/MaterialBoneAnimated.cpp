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

#include "../../OpenGLWrapper/include/openglwrapper/Shader.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VAO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/basic_mesh_loader/Mesh.hpp"

#include "../../include/quickgl/pipelines/PipelineBoneAnimated.hpp"
#include "../../include/quickgl/MeshManager.hpp"
#include "../../include/quickgl/cameras/Camera.hpp"
#include "../../include/quickgl/Engine.hpp"
#include "../../include/quickgl/IndirectDrawBufferGenerator.hpp"

#include "../../include/quickgl/materials/MaterialBoneAnimated.hpp"

namespace qgl {
	MaterialBoneAnimated::MaterialBoneAnimated(
			std::shared_ptr<PipelineBoneAnimated> pipeline) :
		Material(pipeline) {
		this->pipeline = pipeline;
	}
	
	MaterialBoneAnimated::~MaterialBoneAnimated() {
	}
	
	void MaterialBoneAnimated::Init() {
		// init shader
		renderShader = std::make_unique<gl::Shader>();
		if(renderShader->Compile(VERTEX_SHADER_SOURCE, "", FRAGMENT_SHADER_SOURCE))
			exit(31);
		
		// init vao
		vao = std::make_unique<gl::VAO>(gl::TRIANGLES);
		vao->Init();
		gl::VBO& vbo = pipeline->meshManager->GetVBO();
		
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_pos"), 3, gl::FLOAT, false, 0, 0);
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_color"), 4, gl::UNSIGNED_BYTE, true, 12, 0);
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_normal"), 4, gl::BYTE, true, 16, 0);
		
		vao->SetAttribPointer(     vbo, renderShader->GetAttributeLocation("in_weight"), 4, gl::UNSIGNED_BYTE, true, 20, 0);
		vao->SetIntegerAttribPointer(vbo, renderShader->GetAttributeLocation("in_bones"), 4, gl::UNSIGNED_BYTE, 24, 0);
		
		// init animation state vertex attribute
		vao->SetIntegerAttribPointer(pipeline->perEntityAnimationState.Vbo(),
				renderShader->GetAttributeLocation("in_animationState"),
				4, gl::UNSIGNED_INT, 12, 1);
		
		// init model matrix
		gl::VBO& modelVbo = pipeline->transformMatrices.Vbo();
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+0, 4, gl::FLOAT, false, 0, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+1, 4, gl::FLOAT, false, 16, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+2, 4, gl::FLOAT, false, 32, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+3, 4, gl::FLOAT, false, 48, 1);
		vao->BindElementBuffer(pipeline->meshManager->GetEBO(), gl::UNSIGNED_INT);
		
		// get shader uniform locations
		PROJECTION_VIEW_LOCATION =
			renderShader->GetUniformLocation("projectionView");
	}
	
	void MaterialBoneAnimated::Destroy() {
		pipeline = nullptr;
		if(vao)
			vao->Delete();
		if(renderShader)
			renderShader->Destroy();
		vao = nullptr;
		renderShader = nullptr;
	}
	
	std::string MaterialBoneAnimated::GetName() const {
		return "MeterialStatic";
	}
	
	std::shared_ptr<Pipeline> MaterialBoneAnimated::GetPipeline() {
		return pipeline;
	}
	
	void MaterialBoneAnimated::RenderPass(std::shared_ptr<Camera> camera,
			std::shared_ptr<gl::VBO> entitiesToRender,
			gl::VBO& meshInfo,
			uint32_t entitiesCount) {
		if(entitiesCount == 0) {
			return;
		}
	
		vao->Bind();
		renderShader->Use();
		
		glm::mat4 pv = camera->GetPerspectiveMatrix()
			* camera->GetViewMatrix();
		renderShader->SetMat4(PROJECTION_VIEW_LOCATION, pv);
		
		for(uint32_t offset=0; offset<entitiesCount;) {
			uint32_t generatedEntities = 0;
			std::shared_ptr<gl::VBO> indirectBuffer
				= engine->GetIndirectDrawBufferGenerator()
					->Generate(*entitiesToRender, meshInfo, entitiesCount-offset,
							offset, generatedEntities);
			offset += generatedEntities;
			
			renderShader->Use();
			vao->BindIndirectBuffer(*indirectBuffer);
			vao->DrawMultiElementsIndirect(nullptr,
					generatedEntities);
		}
		vao->Unbind();
		gl::Shader::Unuse();
	}
	
	const char* MaterialBoneAnimated::VERTEX_SHADER_SOURCE = R"(
#version 420 core

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
	
	const char* MaterialBoneAnimated::FRAGMENT_SHADER_SOURCE = R"(
#version 420 core

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
	
}

