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

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace gl {
	class VBO;
	class VAO;
	class Shader;
}

namespace qgl {
	
	class MeshManager;
	class Camera;
	
	class Pipeline {
	public:
		
		Pipeline();
		virtual ~Pipeline();
		
		virtual void Initialize();
		
		virtual uint32_t CreateEntity() = 0;
		virtual void DeleteEntity(uint32_t entityId) = 0;
		
		virtual void SetEntityMesh(uint32_t entityId, uint32_t meshId) = 0;
		void SetEntityMeshByName(uint32_t entityId, const char* meshName);
		virtual void SetEntityTransformsQuat(uint32_t entityId, glm::vec3 pos={0,0,0},
				glm::quat rot={0,0,0,1}, glm::vec3 scale={1,1,1}) = 0;
		void SetEntityTransformsEuler(uint32_t entityId, glm::vec3 pos={0,0,0},
				glm::vec3 eulerRot={0,0,0}, glm::vec3 scale={1,1,1});
		
		virtual uint32_t DrawStage(std::shared_ptr<Camera> camera,
				uint32_t stageId) = 0; // returns number of stages left for drawing
		
		inline std::shared_ptr<MeshManager> GetMeshManager() { return meshManager; }
		
		virtual void FlushDataToGPU() = 0;
		
	protected:
		
		virtual std::shared_ptr<MeshManager> CreateMeshManager() = 0;
		
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

