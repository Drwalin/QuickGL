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
#include <functional>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "../util/Log.hpp"
#include "../util/RenderStageComposer.hpp"

namespace gl {
	class VBO;
	class VAO;
	class Shader;
}

namespace qgl {
	
	struct Stage;
	
	class MeshManager;
	class Camera;
	class Engine;
	class Engine;
	class Material;
	
	class Pipeline : public std::enable_shared_from_this<Pipeline> {
	public:
		
		Pipeline(std::shared_ptr<Engine> engine);
		virtual ~Pipeline();
		
		virtual void Init(); // init buffers and prepare stages
		virtual void Destroy();

		inline  PipelineStagesScheduler& GetStageScheduler() { return stagesScheduler; }
		
		void SetPipelineId(uint32_t newId);
		uint32_t GetPipelineId() const;
		virtual std::string GetName() const = 0;
		
	public:
		
		virtual uint32_t CreateEntity() = 0;
		virtual void DeleteEntity(uint32_t entityId) = 0;
		virtual uint32_t GetEntitiesCount() const = 0;
		virtual uint32_t GetEntitiesToRender() const = 0; // losely defined (returns
													// aproximate number of
													// rendered objects),
													// implementation dependant
		
		virtual void SetEntityMesh(uint32_t entityId, uint32_t meshId) = 0;
		void SetEntityMeshByName(uint32_t entityId, const char* meshName);
		virtual void SetEntityTransformsQuat(uint32_t entityId,
				glm::vec3 pos={0,0,0}, glm::quat rot={0,0,0,1},
				glm::vec3 scale={1,1,1}) = 0;
		void SetEntityTransformsEuler(uint32_t entityId, glm::vec3 pos={0,0,0},
				glm::vec3 eulerRot={0,0,0}, glm::vec3 scale={1,1,1});
		
		inline std::shared_ptr<MeshManager> GetMeshManager() { return meshManager; }
		
		virtual uint32_t GetEntityOffset(uint32_t entityId) const = 0;
		
		inline std::shared_ptr<Engine> GetEngine() { return engine; }
		
	protected:
		
		virtual std::shared_ptr<MeshManager> CreateMeshManager() = 0;
		
	protected:
		
		uint32_t pipelineId;
		
		std::shared_ptr<Engine> engine;
		
		std::shared_ptr<MeshManager> meshManager;
		
		PipelineStagesScheduler stagesScheduler;
		std::shared_ptr<Material> material;
	};
}

#endif

