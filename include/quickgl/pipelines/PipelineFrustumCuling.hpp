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

#ifndef QUICKGL_PIPELINE_FRUSTUM_CULLING_HPP
#define QUICKGL_PIPELINE_FRUSTUM_CULLING_HPP

#include <glm/glm.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/vector_relational.hpp>
#include <glm/mat4x4.hpp>

#include "../util/BufferedVBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Sync.hpp"

#include "PipelineIdsManagedBase.hpp"

namespace qgl {
	
	class PipelineFrustumCulling : public PipelineIdsManagedBase {
	public:
		
		PipelineFrustumCulling(std::shared_ptr<Engine> engine);
		virtual ~PipelineFrustumCulling();
		
		virtual uint32_t GetEntitiesToRender() const override;
		
		virtual void Init() override;
		virtual void Destroy() override;
		
	protected:
		
		void UpdateFrustumCullingData(std::shared_ptr<Camera> camera);
		void UpdateClippingPlanesOfCameraToGPU(std::shared_ptr<Camera> camera);
		void PerformFrustumCulling(std::shared_ptr<Camera> camera);
		void FetchFrustumCulledEntitiesCount(std::shared_ptr<Camera> camera);
		bool CanExecuteFetchFrustumCulledEntitiesCount(std::shared_ptr<Camera> camera);
		void GenerateIndirectDrawCommandBuffer(std::shared_ptr<Camera> camera);
		
		uint32_t UNIFORM_LOCATION_DEPTH_TEXTURE;
		
	protected:
		
		uint32_t frustumCulledEntitiesCount;
		
	protected:
		
		std::shared_ptr<gl::VBO> indirectDrawBuffer;
		
		std::unique_ptr<gl::Shader> frustumCullingShader;
		std::shared_ptr<gl::VBO> frustumCulledIdsBuffer;
		std::shared_ptr<gl::VBO> frustumCulledIdsCountAtomicCounter;
		std::shared_ptr<gl::VBO> frustumCulledIdsCountAtomicCounterAsyncFetch;
		std::shared_ptr<gl::VBO> clippingPlanes;
		
		static const char* FRUSTUM_CULLING_COMPUTE_SHADER_SOURCE;
		
		gl::Sync syncFrustumCulledEntitiesCountReadyToFetch;
		
		uint32_t *mappedPointerToentitiesCount;
		
		uint32_t objectsPerInvocation;
	};
}

#endif

