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
#include "../../include/quickgl/util/RenderStageComposer.hpp"

#include "../../include/quickgl/pipelines/PipelineIdsManagedBase.hpp"

namespace qgl {
	PipelineIdsManagedBase::PipelineIdsManagedBase(
		std::shared_ptr<Engine> engine) :
			Pipeline(engine), perEntityMeshInfo(engine),
			perEntityMeshInfoBoundingSphere(engine), transformMatrices(engine) {
	}
	
	PipelineIdsManagedBase::~PipelineIdsManagedBase() {
	}
	
	uint32_t PipelineIdsManagedBase::CreateEntity() {
		uint32_t id = entityBufferManager->GetNewEntity();
		return id;
	}
	
	void PipelineIdsManagedBase::DeleteEntity(uint32_t entityId) {
		entityBufferManager->FreeEntity(entityId);
	}
	
	uint32_t PipelineIdsManagedBase::GetEntitiesCount() const {
		return entityBufferManager->Count();
	}
	
	void PipelineIdsManagedBase::Initialize() {
		entityBufferManager = std::make_shared<EntityBufferManager>(engine,
				shared_from_this());
		
		Pipeline::Initialize();
		perEntityMeshInfo.Init();
		perEntityMeshInfoBoundingSphere.Init();
		transformMatrices.Init();
		entityBufferManager->Init();
		
		entityBufferManager->AddManagedSparselyUpdateVBO(&perEntityMeshInfo);
		entityBufferManager->AddManagedSparselyUpdateVBO(&perEntityMeshInfoBoundingSphere);
		entityBufferManager->AddManagedSparselyUpdateVBO(&transformMatrices);
	}
	
	void PipelineIdsManagedBase::SetEntityMesh(uint32_t entityId,
			uint32_t meshId) {
		entityId = GetEntityOffset(entityId);
		PerEntityMeshInfo info;
		meshManager->GetMeshIndices(meshId, info.elementsStart,
				info.elementsCount);
		perEntityMeshInfo.SetValue(info, entityId);
		
		PerEntityMeshInfoBoundingSphere info2;
		meshManager->GetMeshBoundingSphere(meshId, info2.boundingSphereCenterOffset,
				info2.boundingSphereRadius);
		perEntityMeshInfoBoundingSphere.SetValue(info2, entityId);
	}
	
	void PipelineIdsManagedBase::SetEntityTransformsQuat(uint32_t entityId,
			glm::vec3 pos, glm::quat rot, glm::vec3 scale) {
		entityId = GetEntityOffset(entityId);
		glm::mat4 t = glm::translate(glm::scale(
					glm::mat4_cast(rot), scale), pos);
		transformMatrices.SetValue(t, entityId);
	}
	
	uint32_t PipelineIdsManagedBase::GetEntityOffset(uint32_t entityId) const {
		return entityBufferManager->GetOffsetOfEntity(entityId);
	}
	
	void PipelineIdsManagedBase::FlushDataToGPU() {
		perEntityMeshInfo.UpdateVBO();
		perEntityMeshInfoBoundingSphere.UpdateVBO();
		transformMatrices.UpdateVBO();
	}
	
	void PipelineIdsManagedBase::GenerateRenderStages(
			std::vector<Stage>& stages) {
		Pipeline::GenerateRenderStages(stages);
		stages.emplace_back(
			"Updating EntityBufferManager",
			STAGE_GLOBAL,
			[=](std::shared_ptr<Camera> camera) {
				entityBufferManager->UpdateBuffers();
			}
		);
	}
}

