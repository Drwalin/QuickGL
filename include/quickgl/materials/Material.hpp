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

#ifndef QUICKGL_MATERIAL_HPP
#define QUICKGL_MATERIAL_HPP

#include <memory>
#include <string>

namespace gl {
	class VBO;
	class VAO;
	class Shader;
}

namespace qgl {
	class Camera;
	class Pipeline;
	class Engine;
	class MeshManager;
	
	class Material : std::enable_shared_from_this<Material> {
	public:
		
		Material(std::shared_ptr<Pipeline> pipeline);
		virtual ~Material();
		
		virtual void Init();
		virtual void Destroy();
		
		virtual std::shared_ptr<Pipeline> GetPipeline() = 0;
		
		virtual std::string GetName() const = 0;
		
		virtual void RenderPass(std::shared_ptr<Camera> camera,
				std::shared_ptr<gl::VBO> entitiesToRender,
				uint32_t entitiesCount) = 0;
		
	protected:
		
		static std::shared_ptr<gl::VBO> GenerateIndirectDrawBuffer(
			std::shared_ptr<gl::VBO> entitiesToRender,
			uint32_t entitiesCount);
		
		static std::shared_ptr<gl::Shader> GetIndirectDrawBufferGenerator();
		
	protected:
		
		std::shared_ptr<Engine> engine;
	};
}

#endif

