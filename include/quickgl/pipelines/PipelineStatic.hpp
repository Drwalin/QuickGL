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
#include "../util/BufferedVBO.hpp"

namespace qgl {
	
	class PipelineStatic final : public PipelineIdsManagedBase {
	public:
		
		PipelineStatic();
		virtual ~PipelineStatic();
		
		virtual uint32_t CreateEntity() override;
		
		virtual void Initialize() override;
		
		void SetEntityPos(uint32_t entityId, glm::vec3 pos);
		void SetEntityTransform(uint32_t entityId, const glm::mat4& matrix);
		void SetEntityRotation(uint32_t entityId, glm::quat rotation);
		void SetEntityScale(uint32_t entityId, glm::vec3 scale);
		
		virtual uint32_t DrawStage(std::shared_ptr<Camera> camera,
				uint32_t stageId) override;
		
		virtual void FlushDataToGPU() override;
		
	protected:
		
		virtual std::shared_ptr<MeshManager> CreateMeshManager() override;
		
		struct DrawElementsIndirectCommand {
			uint32_t count;
			uint32_t instanceCount;
			uint32_t firstIndex;
			int32_t  baseVertex;
			uint32_t baseInstance;
		};
		
		TypedVBO<glm::mat4> transformMatrices;
		TypedVBO<DrawElementsIndirectCommand> vboIndirectDrawBuffer;
		
		std::unique_ptr<gl::VAO> vao;
		std::unique_ptr<gl::Shader> renderShader;
		
		// move this to derived class
// 		gl::VBO* vboFrustumCulledEntityIds;
// 		gl::VBO* vboAtomicCounterForCulledEntities;
	};
}

#endif

