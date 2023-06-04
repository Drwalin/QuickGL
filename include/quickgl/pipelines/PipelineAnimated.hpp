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

#ifndef QUICKGL_PIPELINE_ANIMATED_HPP
#define QUICKGL_PIPELINE_ANIMATED_HPP

#include <glm/glm.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/vector_relational.hpp>
#include <glm/mat4x4.hpp>

#include "../util/BufferedVBO.hpp"
#include "../AnimatedMeshManager.hpp"

#include "PipelineFrustumCuling.hpp"

namespace qgl {
	
	class PipelineAnimated final : public PipelineFrustumCulling {
	public:
		
		PipelineAnimated();
		virtual ~PipelineAnimated();
		
		virtual void Initialize() override;
		
		virtual uint32_t CreateEntity() override;
		
	public:
		
		virtual void AppendRenderStages(std::vector<StageFunction>& stages) override;
		
		void SetAnimationState(uint32_t entityId, uint32_t animationId,
				float timeOffset, bool enableUpdateTime,
				uint32_t animationIdAfter, bool continueNextAnimation);
		
	protected:
		
		virtual std::shared_ptr<MeshManager> CreateMeshManager() override;
		virtual uint32_t FlushDataToGPU(uint32_t stageId) override;
		
	private:
		
		struct AnimatedState {
			uint32_t animationId;
			uint32_t animationIdAfter;
			uint32_t flags; // 1 - continueNextAnimation, 2 - updateTime
			
			uint32_t firstMatrixFrameCurrent;
			uint32_t firstMatrixFrameNext;
			float interpolationFactor;
			
			float timeOffset;
			float lastAccessTimeStamp;
		};
		
		ManagedSparselyUpdatedVBO<AnimatedState> perEntityAnimationState;
		
		std::unique_ptr<gl::VAO> vao;
		std::unique_ptr<gl::Shader> renderShader;
		std::unique_ptr<gl::Shader> updateAnimationShader;
		
		std::shared_ptr<AnimatedMeshManager> animatedMeshManager;
		
		static const char* VERTEX_SHADER_SOURCE;
		static const char* FRAGMENT_SHADER_SOURCE;
		static const char* UPDATE_ANIMATION_SHADER_SOURCE;
	};
}

#endif

