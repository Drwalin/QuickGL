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

#ifndef QUICKGL_PIPELINE_IDS_MANAGED_BASE_HPP
#define QUICKGL_PIPELINE_IDS_MANAGED_BASE_HPP

#include "../util/IdsManager.hpp"
#include "../util/BufferedVBO.hpp"

#include "../util/ManagedSparselyUpdatedVBO.hpp"

#include "Pipeline.hpp"

namespace qgl {
	class PipelineIdsManagedBase : public Pipeline {
	public:
		
		PipelineIdsManagedBase();
		virtual ~PipelineIdsManagedBase();
		
		virtual uint32_t CreateEntity() override;
		virtual void DeleteEntity(uint32_t entityId) override;
		
		virtual void Initialize() override;
		
		virtual void SetEntityMesh(uint32_t entityId, uint32_t meshId) override;
		virtual void SetEntityTransformsQuat(uint32_t entityId,
				glm::vec3 pos={0,0,0}, glm::quat rot={0,0,0,1},
				glm::vec3 scale={1,1,1}) override;
		
	protected:
		
		virtual uint32_t FlushDataToGPU(uint32_t stageId) override;
		
	protected:

		struct PerEntityMeshInfo {
			uint32_t elementsStart;
			uint32_t elementsCount;
		};
		
		struct PerEntityMeshInfoBoundingSphere {
			float boundingSphereCenterOffset[3];
			float boundingSphereRadius;
		};
		
		ManagedSparselyUpdatedVBO<PerEntityMeshInfo> perEntityMeshInfo;
		
		ManagedSparselyUpdatedVBO<PerEntityMeshInfoBoundingSphere> perEntityMeshInfoBoundingSphere;
		
		ManagedSparselyUpdatedVBO<glm::mat4> transformMatrices;
		
		IdsManagerVBOManaged idsManager;
	};
}

#endif

