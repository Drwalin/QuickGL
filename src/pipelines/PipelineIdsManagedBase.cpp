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

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"

#include "../../include/quickgl/MeshManager.hpp"

#include "../../include/quickgl/pipelines/PipelineIdsManagedBase.hpp"

namespace qgl {
	PipelineIdsManagedBase::PipelineIdsManagedBase() {
	}
	
	PipelineIdsManagedBase::~PipelineIdsManagedBase() {
	}
	
	uint32_t PipelineIdsManagedBase::CreateEntity() {
		uint32_t id = idsManager.GetNewId();
		if(id >= perEntityMeshInfo.Count()) {
			perEntityMeshInfo.Resize(id + 100);
			idsBuffer->Resize(id+100);
		}
		return id;
	}
	
	void PipelineIdsManagedBase::DeleteEntity(uint32_t entityId) {
		idsManager.FreeId(entityId);
	}
	
	void PipelineIdsManagedBase::Initialize() {
		Pipeline::Initialize();
		idsBuffer = std::make_shared<gl::VBO>(sizeof(uint32_t),
					gl::ARRAY_BUFFER, gl::DYNAMIC_DRAW);
		idsBuffer->Generate(nullptr, 128);
		perEntityMeshInfo.Resize(128);
		transformMatrices.Init();
	}
	
	void PipelineIdsManagedBase::SetEntityMesh(uint32_t entityId,
			uint32_t meshId) {
		PerEntityMeshInfo info;
		meshManager->GetMeshIndices(meshId, info.elementsStart,
				info.elementsCount);
		perEntityMeshInfo.SetValue(info, entityId);
	}
	
	void PipelineIdsManagedBase::SetEntityTransformsQuat(uint32_t entityId,
			glm::vec3 pos, glm::quat rot, glm::vec3 scale) {
		transformMatrices.SetValue(glm::translate(glm::scale(
					glm::mat4_cast(rot), scale), pos), entityId);
	}
	
	void PipelineIdsManagedBase::FlushDataToGPU() {
		idsBuffer->Update(idsManager.GetArrayOfUsedIds(), 0,
				idsBuffer->VertexSize()*idsManager.GetArraySize());
		perEntityMeshInfo.UpdateVBO();
		transformMatrices.UpdateVBO();
	}
}

