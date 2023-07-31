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

#ifndef QUICKGL_INDIRECT_DRAW_BUFFER_GENERATOR_HPP
#define QUICKGL_INDIRECT_DRAW_BUFFER_GENERATOR_HPP

#include <memory>

namespace gl {
	class VBO;
	class Shader;
}

namespace qgl {
	class Engine;
	class DeltaVboManager;
	
	class IndirectDrawBufferGenerator final {
	public:
		
		IndirectDrawBufferGenerator(std::shared_ptr<Engine> engine);
		~IndirectDrawBufferGenerator();
		
		void Init();
		void Destroy();
		
		std::shared_ptr<gl::VBO> Generate(
				gl::VBO& entitiesToRender,
				gl::VBO& meshIdsPerEntity,
				gl::VBO& meshInfo,
				uint32_t entitiesCount,
				uint32_t entitiesOffset,
				uint32_t& generatedCount);
		
		void Generate(
				gl::VBO& entitiesToRender,
				gl::VBO& meshIdsPerEntity,
				gl::VBO& meshInfo,
				gl::VBO& indirectDrawBuffer,
				uint32_t entitiesCount,
				uint32_t entitiesOffset);
		
	private:
		
		std::shared_ptr<gl::Shader> shader;
		std::shared_ptr<Engine> engine;
		
		uint32_t ENTITIES_COUNT_LOCATION;
		uint32_t ENTITIES_OFFSET_LOCATION;
		
		static const char* INDIRECT_DRAW_BUFFER_COMPUTE_SHADER_SOURCE;
	};
}

#endif

