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
	GL_CHECK_PUSH_ERROR;
		Pipeline::Initialize();
	GL_CHECK_PUSH_ERROR;
		idsBuffer = std::make_shared<gl::VBO>(sizeof(uint32_t),
					gl::ARRAY_BUFFER, gl::DYNAMIC_DRAW);
	GL_CHECK_PUSH_ERROR;
		idsBuffer->Generate(nullptr, 128);
	GL_CHECK_PUSH_ERROR;
		perEntityMeshInfo.Resize(128);
	GL_CHECK_PUSH_ERROR;
	}
	
	void PipelineIdsManagedBase::SetEntityMesh(uint32_t entityId,
			uint32_t meshId) {
		PerEntityMeshInfo& info = perEntityMeshInfo[entityId];
		meshManager->GetMeshIndices(meshId, info.elementsStart,
				info.elementsCount);
	}
	
	void PipelineIdsManagedBase::FlushDataToGPU() {
		idsBuffer->Update(idsManager.GetArrayOfUsedIds(), 0,
				idsBuffer->VertexSize()*idsManager.GetArraySize());
		perEntityMeshInfo.UpdateVertices(0, idsManager.GetArraySize());
	}
}

