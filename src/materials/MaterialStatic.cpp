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

#include "../../include/quickgl/pipelines/PipelineStatic.hpp"
#include "../../include/quickgl/MeshManager.hpp"
#include "../../include/quickgl/Engine.hpp"
#include "../../include/quickgl/IndirectDrawBufferGenerator.hpp"
#include "../../include/quickgl/cameras/Camera.hpp"

#include "../../include/quickgl/materials/MaterialStatic.hpp"

namespace qgl {
	MaterialStatic::MaterialStatic(std::shared_ptr<PipelineStatic> pipeline) :
		Material(pipeline) {
		this->pipeline = pipeline;
	}
	
	MaterialStatic::~MaterialStatic() {
	}
	
	void MaterialStatic::Init() {
		// init shaders
		renderShader = std::make_unique<gl::Shader>();
		if(renderShader->Compile(VERTEX_SHADER_SOURCE, "", FRAGMENT_SHADER_SOURCE))
			exit(31);
		// 
		// init vao
		vao = std::make_unique<gl::VAO>(gl::TRIANGLES);
		vao->Init();
		gl::VBO& vbo = pipeline->GetMeshManager()->GetVBO();
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_pos"), 3, gl::FLOAT, false, 0, 0);
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_color"), 4, gl::UNSIGNED_BYTE, true, 12, 0);
		vao->SetAttribPointer(vbo, renderShader->GetAttributeLocation("in_normal"), 4, gl::BYTE, true, 16, 0);
		
		// init model matrix
		gl::VBO& modelVbo = pipeline->transformMatrices.Vbo();
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+0, 4, gl::FLOAT, false, 0, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+1, 4, gl::FLOAT, false, 16, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+2, 4, gl::FLOAT, false, 32, 1);
		vao->SetAttribPointer(modelVbo, renderShader->GetAttributeLocation("model")+3, 4, gl::FLOAT, false, 48, 1);
		vao->BindElementBuffer(pipeline->GetMeshManager()->GetEBO(), gl::UNSIGNED_INT);
		
		// get shader uniform locations
		PROJECTION_VIEW_LOCATION =
			renderShader->GetUniformLocation("projectionView");
	}
	
	void MaterialStatic::Destroy() {
		pipeline = nullptr;
		if(vao)
			vao->Delete();
		if(renderShader)
			renderShader->Destroy();
		vao = nullptr;
		renderShader = nullptr;
	}
	
	std::string MaterialStatic::GetName() const {
		return "MeterialStatic";
	}
	
	std::shared_ptr<Pipeline> MaterialStatic::GetPipeline() {
		return pipeline;
	}
	
	void MaterialStatic::RenderPassIndirect(std::shared_ptr<Camera> camera,
			gl::VBO& indirectBuffer,
			uint32_t entitiesCount) {
		if(entitiesCount == 0) {
			return;
		}
		
		vao->Bind();
		renderShader->Use();
		
		glm::mat4 pv = camera->GetPerspectiveMatrix()
			* camera->GetViewMatrix();
		renderShader->SetMat4(PROJECTION_VIEW_LOCATION, pv);
		
		renderShader->Use();
		vao->BindIndirectBuffer(indirectBuffer);
		vao->DrawMultiElementsIndirect(nullptr, entitiesCount);
			
		gl::Shader::Unuse();
		vao->Unbind();
	}
	
	const char* MaterialStatic::VERTEX_SHADER_SOURCE = R"(
#version 420 core

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
	
	const char* MaterialStatic::FRAGMENT_SHADER_SOURCE = R"(
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

