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

#include "../util/BufferedVBO.hpp"

#include "PipelineFrustumCuling.hpp"

namespace qgl {
	
	class PipelineStatic final : public PipelineFrustumCulling {
	public:
		
		PipelineStatic(std::shared_ptr<Engine> engine);
		virtual ~PipelineStatic();
		
		virtual void Init() override;
		virtual void Destroy() override;
		
		virtual std::string GetName() const override;
		
	protected:
		
		virtual std::shared_ptr<MeshManager> CreateMeshManager() override;
		
		friend class MaterialStatic;
		
	private:
		
		std::unique_ptr<gl::VAO> vao;
		std::unique_ptr<gl::Shader> renderShader;
		
		static const char* VERTEX_SHADER_SOURCE;
		static const char* FRAGMENT_SHADER_SOURCE;
	};
}

#endif

