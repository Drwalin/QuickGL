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

#include "../../include/quickgl/pipelines/Pipeline.hpp"

namespace qgl {
	
	Pipeline::Pipeline() {}
	
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
	
	void Pipeline::AppendRenderStages(std::vector<StageFunction>& stages) {
		stages.emplace_back([this](std::shared_ptr<Camera> camera){
				this->FlushDataToGPU(0);
				gl::Flush();
			});
		stages.emplace_back([this](std::shared_ptr<Camera> camera){
				this->FlushDataToGPU(1);
			});
	}
}

