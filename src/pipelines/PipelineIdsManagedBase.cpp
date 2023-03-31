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
		return id;
	}
	
	void PipelineIdsManagedBase::DeleteEntity(uint32_t entityId) {
		idsManager.FreeId(entityId);
	}
	
	void PipelineIdsManagedBase::Initialize() {
		Pipeline::Initialize();
		perEntityMeshInfo.Init();
		transformMatrices.Init();
		idsManager.InitVBO();
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
		glm::mat4 t = glm::translate(glm::scale(
					glm::mat4_cast(rot), scale), pos);
		transformMatrices.SetValue(t, entityId);
	}
	
	uint32_t PipelineIdsManagedBase::FlushDataToGPU(uint32_t stageId) {
		uint32_t ret = 0;
		ret = std::max(ret, perEntityMeshInfo.UpdateVBO(stageId));
		ret = std::max(ret, idsManager.UpdateVBO(stageId));
		ret = std::max(ret, transformMatrices.UpdateVBO(stageId));
		return ret;
	}
}

