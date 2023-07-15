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
	
	class Material {
	public:
		
		Material(std::shared_ptr<Pipeline> pipeline);
		virtual ~Material();
		
		virtual std::string GetName() const = 0;
		
		virtual void Initialize();
		
		void GenerateIndirectDrawBuffer(
				std::shared_ptr<gl::VBO> entitiesToRender,
				uint32_t entitiesCount);
		
		virtual void RenderPassIndirect(std::shared_ptr<Camera> camera,
				uint32_t entitiesCount) = 0;
		
		virtual std::shared_ptr<MeshManager> CreateMeshManager() = 0;
		
	protected:
		
		std::shared_ptr<Pipeline> pipeline;
		std::shared_ptr<Engine> engine;
		
		std::shared_ptr<gl::VAO> vao;
		std::shared_ptr<gl::Shader> renderShader;
		
		std::shared_ptr<gl::VBO> indirectDrawBuffer;
		static std::shared_ptr<gl::Shader> idirectDrawBufferGenerator;
	};
}

#endif

