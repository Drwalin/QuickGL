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

#include "../../include/quickgl/pipelines/Pipeline.hpp"

namespace qgl {
	
	Pipeline::Pipeline(std::shared_ptr<Engine> engine) : engine(engine) {}
	
	Pipeline::~Pipeline() {}
	
	void Pipeline::Initialize() {
		meshManager = CreateMeshManager();
	}
	
	void Pipeline::SetEntityMeshByName(uint32_t entityId, const char* meshName) {
		SetEntityMesh(entityId, meshManager->GetMeshIdByName(meshName));
	}
	
	void Pipeline::SetEntityTransformsEuler(uint32_t entityId, glm::vec3 pos,
			glm::vec3 eulerRot, glm::vec3 scale) {
		SetEntityTransformsQuat(entityId, pos, glm::quat(eulerRot), scale);
	}
	
	void Pipeline::GenerateRenderStages(std::vector<struct Stage>& stages) {
		stages.emplace_back(
			"Flush data to GPU",
			STAGE_GLOBAL,
			[this](std::shared_ptr<Camera> camera) {
				this->FlushDataToGPU();
				gl::Flush();
			}
		);
	}
	
	void Pipeline::SetPipelineId(uint32_t newId) {
		pipelineId = newId;
	}
	
	uint32_t Pipeline::GetPipelineId() const {
		return pipelineId;
	}
}

