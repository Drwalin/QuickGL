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

#ifndef QUICKGL_PIPELINE_HPP
#define QUICKGL_PIPELINE_HPP

#include <cinttypes>

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

namespace gl {
	class VBO;
	class VAO;
}

namespace qgl {
	
	class MeshManager;
	
	class Pipeline {
	public:
		
		Pipeline();
		virtual ~Pipeline();
		
		virtual void Initialize() = 0;
		
		virtual uint32_t CreateEntity() = 0;
		virtual void DeleteEntity(uint32_t entityId) = 0;
		
// 		virtual void SetEntityPos(uint32_t entityId, glm::vec3 pos) = 0;
// 		virtual void SetEntityTransform(uint32_t entityId, const glm::mat4& matrix) = 0;
// 		virtual void SetEntityRotationScale(uint32_t entityId, const glm::mat3& roration) = 0;
// 		virtual void SetEntityRotation(uint32_t entityId, glm::quat roration) = 0;
// 		virtual void SetEntityScale(uint32_t entityId, glm::vec3 scale) = 0;
// 		virtual void SetEntityData(uint32_t entityId, const void* data) = 0;
		
		virtual void SetEntityMesh(uint32_t entityId, uint32_t meshId) = 0;
		
		virtual uint32_t DrawStage(uint32_t stageId) = 0; // returns number of stages left for drawing
		
		inline std::shared_ptr<MeshManager> GetMeshManager() { return meshManager; }
		
	protected:
		
		virtual void FlushMeshManagerStateChangesToGPU() = 0;
		virtual void FlushDataToGPU() = 0;
		
	protected:
		
		std::shared_ptr<MeshManager> meshManager;
		
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

