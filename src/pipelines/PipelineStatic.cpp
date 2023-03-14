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

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VAO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Shader.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/basic_mesh_loader/Mesh.hpp"

#include "../../include/quickgl/MeshManager.hpp"

#include "../../include/quickgl/pipelines/PipelineStatic.hpp"

namespace qgl {
	PipelineStatic::PipelineStatic() {
		transformMatrices.Resize(100);
	}
	
	PipelineStatic::~PipelineStatic() {
	}
	
	uint32_t PipelineStatic::CreateEntity() {
		uint32_t id = PipelineIdsManagedBase::CreateEntity();
		if(id >= transformMatrices.Count()) {
			transformMatrices.Resize(id + 100);
			vboIndirectDrawBuffer.Resize(id + 100);
		}
		return id;
	}
	
	void PipelineStatic::Initialize() {
		PipelineIdsManagedBase::Initialize();
		renderShader = std::make_unique<gl::Shader>();
		vao = std::make_unique<gl::VAO>(gl::TRIANGLES);
		throw "PipelineStatic::Initialize() is not implemented.";
		// TODO: implement Shader loading, VAO attributes
	}
	
	void PipelineStatic::SetEntityPos(uint32_t entityId, glm::vec3 pos) {
		glm::mat4& m = transformMatrices[entityId];
		glm::vec3 _scale, _pos, _skew; 
		glm::vec4 _persp;
		glm::quat _rot;
		glm::decompose(m, _scale, _rot, _pos, _skew, _persp);
		m = glm::translate(glm::scale(glm::mat4_cast(_rot), _scale), pos);
	}
	
	void PipelineStatic::SetEntityTransform(uint32_t entityId,
			const glm::mat4& matrix) {
		transformMatrices[entityId] = matrix;
	}
	
	void PipelineStatic::SetEntityRotation(uint32_t entityId,
			glm::quat rotation) {
		glm::mat4& m = transformMatrices[entityId];
		glm::vec3 _scale, _pos, _skew; 
		glm::vec4 _persp;
		glm::quat _rot;
		glm::decompose(m, _scale, _rot, _pos, _skew, _persp);
		m = glm::translate(glm::scale(glm::mat4_cast(rotation), _scale), _pos);
	}
	
	void PipelineStatic::SetEntityScale(uint32_t entityId, glm::vec3 scale) {
		glm::mat4& m = transformMatrices[entityId];
		glm::vec3 _scale, _pos, _skew; 
		glm::vec4 _persp;
		glm::quat _rot;
		glm::decompose(m, _scale, _rot, _pos, _skew, _persp);
		m = glm::translate(glm::scale(glm::mat4_cast(_rot), scale), _pos);
	}
	
	uint32_t PipelineStatic::DrawStage(std::shared_ptr<Camera> camera,
			uint32_t stageId) {
		for(uint32_t i=0; i<idsManager.CountIds(); ++i) {
			uint32_t id = idsManager.GetArrayOfUsedIds()[i];
			vboIndirectDrawBuffer[id] = {
				perEntityMeshInfo[id].elementsCount,
				1,
				perEntityMeshInfo[id].elementsStart,
				0,
				id
			};
		}
		
		vboIndirectDrawBuffer.UpdateVertices(0, idsManager.GetArraySize());
		
		renderShader->Use();
		
		// TODO: set global data for shader
		throw "PipelineStatic::DrawStage() is not fully implemented.";
		
		vao->DrawMultiElementsIndirect(NULL, idsManager.CountIds());
		
		return 0;
	}
	
	
	void PipelineStatic::FlushDataToGPU() {
		PipelineIdsManagedBase::FlushDataToGPU();
		transformMatrices.UpdateVertices(0, idsManager.GetArraySize());
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
}

