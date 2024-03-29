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

#ifndef QUICKGL_PIPELINE_POST_PROCESSING_HPP
#define QUICKGL_PIPELINE_POST_PROCESSING_HPP

#include "Pipeline.hpp"

namespace qgl {
	class PipelinePostProcessing : public Pipeline {
	public:
		
		PipelinePostProcessing(std::shared_ptr<Engine> engine);
		virtual ~PipelinePostProcessing();
		
		virtual void Init() override; // init buffers and prepare stages
		virtual void Destroy() override;
		
		virtual std::string GetName() const override;
		
	public:
		
		virtual uint32_t CreateEntity() override;
		virtual void DeleteEntity(uint32_t entityId) override;
		virtual uint32_t GetEntitiesCount() const override;
		virtual uint32_t GetEntitiesToRender() const override;
		
		virtual void SetEntityMesh(uint32_t entityId, uint32_t meshId) override;
		virtual void SetEntityTransformsQuat(uint32_t entityId,
				glm::vec3 pos={0,0,0}, glm::quat rot=glm::angleAxis(0.0f,glm::vec3(0,1,0)),
				glm::vec3 scale={1,1,1}) override;
		
		virtual uint32_t GetEntityOffset(uint32_t entityId) const override;
		
	protected:
		
		void EmptyRenderStage(std::shared_ptr<Camera> camera);
		
		void DoPostprocessesForCamera(std::shared_ptr<Camera> camera);
		
		virtual std::shared_ptr<MeshManager> CreateMeshManager() override;
	};
}

#endif

