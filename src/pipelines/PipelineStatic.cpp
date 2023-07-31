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

#include <memory>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/matrix_operation.hpp>

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VAO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Shader.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/basic_mesh_loader/Mesh.hpp"

#include "../../include/quickgl/MeshManager.hpp"
#include "../../include/quickgl/cameras/Camera.hpp"
#include "../../include/quickgl/util/RenderStageComposer.hpp"
#include "../../include/quickgl/materials/MaterialStatic.hpp"

#include "../../include/quickgl/pipelines/PipelineStatic.hpp"

namespace qgl {
	PipelineStatic::PipelineStatic(std::shared_ptr<Engine> engine) :
		PipelineFrustumCulling(engine) {
	}
	
	PipelineStatic::~PipelineStatic() {
	}
	
	std::string PipelineStatic::GetName() const {
		return "PipelineStatic";
	}
	
	void PipelineStatic::Init() {
		material = std::make_shared<MaterialStatic>(
				std::dynamic_pointer_cast<PipelineStatic>(
					shared_from_this()));
		
		PipelineFrustumCulling::Init();
		
		material->Init();
		
		stagesScheduler.AddStage(
			"Render static entities",
			STAGE_1_RENDER_PASS_1,
			&PipelineStatic::RenderEntities);
	}
	
	void PipelineStatic::RenderEntities(std::shared_ptr<Camera> camera) {
		material->RenderPassIndirect(camera, *indirectDrawBuffer,
				frustumCulledEntitiesCount);
	}
	
	void PipelineStatic::Destroy() {
		PipelineFrustumCulling::Destroy();
	}
	
	std::shared_ptr<MeshManager> PipelineStatic::CreateMeshManager() {
		static constexpr uint32_t stride
			= 3*sizeof(float)   // pos
			+ 4*sizeof(uint8_t) // color
			+ 4*sizeof(uint8_t) // normal
			;
		
		return std::make_shared<MeshManager>(engine, stride,
			[](std::vector<uint8_t>& buffer, uint32_t offset,
					gl::BasicMeshLoader::Mesh* mesh)->bool {
				mesh->ExtractPos<float>(offset, buffer, 0, stride,
						gl::BasicMeshLoader::ConverterFloatPlain<float, 3>);
				
				if(mesh->color.size() == 0 || mesh->color[0].size() != mesh->pos.size()) {
					mesh->color.resize(std::max<int>(mesh->color.size(), 1));
					mesh->color[0].resize(mesh->pos.size());
					for(auto&c : mesh->color[0]) {
						c = {0,0,0,1};
					}
				}
				mesh->ExtractColor<uint8_t>(offset, buffer, 12, stride,
						gl::BasicMeshLoader::ConverterIntPlainClampScale
							<uint8_t, 255, 0, 255, 4>);
				
				mesh->ExtractNormal(offset, buffer, 16, stride,
						gl::BasicMeshLoader::ConverterIntNormalized
							<uint8_t, 127, 3>);
				
				return true;
			});
	}
}

