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

#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

#include "../../include/quickgl/MeshManager.hpp"
#include "../../include/quickgl/Engine.hpp"
#include "../../include/quickgl/util/RenderStageComposer.hpp"
#include "../../include/quickgl/materials/Material.hpp"
#include "../../include/quickgl/cameras/Camera.hpp"

#include "../../include/quickgl/pipelines/PipelinePostProcessing.hpp"

namespace qgl {
	
	PipelinePostProcessing::PipelinePostProcessing(
			std::shared_ptr<Engine> engine) : Pipeline(engine) {
	}
	
	PipelinePostProcessing::~PipelinePostProcessing() {
	}
	
	void PipelinePostProcessing::Init() {
		Pipeline::Init();
		
		stagesScheduler.AddStage(
				"empty",
				STAGE_GLOBAL,
				&PipelinePostProcessing::EmptyRenderStage);
		
		stagesScheduler.AddStage(
				"Post processing",
				STAGE_POST_PROCESS,
				&PipelinePostProcessing::DoPostprocessesForCamera);
	}
	
	void PipelinePostProcessing::DoPostprocessesForCamera(
			std::shared_ptr<Camera> camera) {
		camera->DoPostprocessing();
	}
	
	void PipelinePostProcessing::EmptyRenderStage(
			std::shared_ptr<Camera> camera) {
	}
	
	void PipelinePostProcessing::Destroy() {
		Pipeline::Destroy();
	}
	
	std::string PipelinePostProcessing::GetName() const {
		return "PipelinePostProcessing";
	}
	
	uint32_t PipelinePostProcessing::CreateEntity() {
		throw "PipelinePostProcessing::CreateEntity() is invalid.";
	}
	
	void PipelinePostProcessing::DeleteEntity(uint32_t entityId) {
		throw "PipelinePostProcessing::DeleteEntity() is invalid.";
	}
	
	uint32_t PipelinePostProcessing::GetEntitiesCount() const {
		return 0;
	}
	
	uint32_t PipelinePostProcessing::GetEntitiesToRender() const {
		return 0;
	}
	
	void PipelinePostProcessing::SetEntityMesh(uint32_t entityId,
			uint32_t meshId) {
		throw "PipelinePostProcessing::SetEntityMesh() is invalid.";
	}
	
	void PipelinePostProcessing::SetEntityTransformsQuat(uint32_t entityId,
			glm::vec3 pos, glm::quat rot, glm::vec3 scale) {
		throw "PipelinePostProcessing::SetEntityTransformsQuat() is invalid.";
	}
	
	uint32_t PipelinePostProcessing::GetEntityOffset(uint32_t entityId) const {
		throw "PipelinePostProcessing::GetEntityOffset() is invalid.";
	}
	
	std::shared_ptr<MeshManager> PipelinePostProcessing::CreateMeshManager() {
		return nullptr;
	}
}

