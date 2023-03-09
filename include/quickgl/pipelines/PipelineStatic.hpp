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

#ifndef QUICKGL_PIPELINE_STATIC_HPP
#define QUICKGL_PIPELINE_STATIC_HPP

#include <glm/glm.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/vector_relational.hpp>
#include <glm/mat4x4.hpp>

#include "PipelineIdsManagedBase.hpp"

namespace qgl {
	
	class PipelineStatic final : public PipelineIdsManagedBase {
	public:
		
		PipelineStatic();
		virtual ~PipelineStatic();
		
		virtual void Initialize() override;
		
		void SetEntityPos(uint32_t entityId, glm::vec3 pos);
		void SetEntityTransform(uint32_t entityId, const glm::mat4& matrix);
		void SetEntityRotationScale(uint32_t entityId, const glm::mat3& roration);
		void SetEntityRotation(uint32_t entityId, glm::quat roration);
		void SetEntityScale(uint32_t entityId, glm::vec3 scale);
		void SetEntityData(uint32_t entityId, const void* data);
		
		virtual void SetEntityMesh(uint32_t entityId, uint32_t meshId) override;
		
		virtual uint32_t DrawStage(std::shared_ptr<Camera> camera,
				uint32_t stageId) override;
		
	protected:
		
		virtual void FlushMeshManagerStateChangesToGPU() override;
		virtual void FlushDataToGPU() override;
		
	protected:
		
// 		struct PerEntityMeshInfo {
// 			uint32_t elementsStart;
// 			uint32_t elementsCount;
// 			uint32_t verticesStart;
// 		};
// 		
// 		std::vector<uint32_t> entityMeshId; // if == 0xFFFFFFFF then entity is not in use
// 		std::vector<uint32_t> unusedEntityIdsStack;
// 		std::vector<PerEntityMeshInfo> entityMeshInfo;
// 		
// 		// move this to derived class
// 		gl::VBO* vboIndirectDrawBuffer;
// 		gl::VBO* vboFrustumCulledEntityIds;
// 		gl::VBO* vboAtomicCounterForCulledEntities;
	};
}

#endif

