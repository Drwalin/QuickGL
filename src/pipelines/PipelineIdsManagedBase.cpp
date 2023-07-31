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
			Pipeline(engine), perEntityMeshIds(engine), transformMatrices(engine) {
	}
	
	PipelineIdsManagedBase::~PipelineIdsManagedBase() {
	}
	
	uint32_t PipelineIdsManagedBase::CreateEntity() {
		uint32_t entity = entityBufferManager->GetNewEntity();
		perEntityMeshIds.SetValue(0, GetEntityOffset(entity));
		return entity;
	}
	
	void PipelineIdsManagedBase::DeleteEntity(uint32_t entityId) {
		SetEntityMesh(entityId, 0);
		entityBufferManager->FreeEntity(entityId);
	}
	
	uint32_t PipelineIdsManagedBase::GetEntitiesCount() const {
		return entityBufferManager->Count();
	}
	
	void PipelineIdsManagedBase::Init() {
		Pipeline::Init();
		
		entityBufferManager = std::make_shared<EntityBufferManager>(engine,
				shared_from_this());
		
		perEntityMeshIds.Init();
		transformMatrices.Init();
		entityBufferManager->Init();
		
		entityBufferManager->AddManagedSparselyUpdatedVBOWithLocal(&perEntityMeshIds);
		entityBufferManager->AddManagedSparselyUpdateVBO(&transformMatrices);
		
		stagesScheduler.AddStage(
				"Update ID manager data",
				STAGE_UPDATE_DATA,
				&PipelineIdsManagedBase::UpdateIDManagerData);
		
		stagesScheduler.AddStage(
				"Updating EntityBufferManager",
				STAGE_GLOBAL,
				&PipelineIdsManagedBase::UpdateEntityBufferManager);
	}
	
	void PipelineIdsManagedBase::UpdateIDManagerData(std::shared_ptr<Camera>) {
		perEntityMeshIds.UpdateVBO();
		transformMatrices.UpdateVBO();
		meshManager->UpdateVbo();
	}
	
	void PipelineIdsManagedBase::UpdateEntityBufferManager(
			std::shared_ptr<Camera>) {
		entityBufferManager->UpdateBuffers();
	}
	
	void PipelineIdsManagedBase::Destroy() {
		perEntityMeshIds.Destroy();
		transformMatrices.Destroy();
		entityBufferManager->Destroy();
		entityBufferManager = nullptr;
		
		Pipeline::Destroy();
	}
	
	void PipelineIdsManagedBase::SetEntityMesh(uint32_t entityId,
			uint32_t meshId) {
		entityId = GetEntityOffset(entityId);
		uint32_t prevMeshId = perEntityMeshIds.GetValue(entityId);
		if(meshId == prevMeshId) {
			return;
		}
		if(prevMeshId) {
			meshManager->ReleaseMeshReference(prevMeshId);
		}
		if(meshId) {
			meshManager->AcquireMeshReference(meshId);
		}
		perEntityMeshIds.SetValue(meshId, entityId);
	}
	
	void PipelineIdsManagedBase::SetEntityTransformsQuat(uint32_t entityId,
			glm::vec3 pos, glm::quat rot, glm::vec3 scale) {
		entityId = GetEntityOffset(entityId);
		glm::mat4 t = glm::translate(glm::mat4(1), pos);
		glm::mat4 r = glm::mat4_cast(rot);
		glm::mat4 s = glm::scale(glm::mat4(1), scale);
		glm::mat4 T = t*r*s;
		transformMatrices.SetValue(T, entityId);
	}
	
	uint32_t PipelineIdsManagedBase::GetEntityOffset(uint32_t entityId) const {
		return entityBufferManager->GetOffsetOfEntity(entityId);
	}
	
}

