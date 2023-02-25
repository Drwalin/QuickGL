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

#ifndef QUICKGL_VERTEX_ATTRIBUTE_CONFIGURATION_HPP
#define QUICKGL_VERTEX_ATTRIBUTE_CONFIGURATION_HPP

#include <cstdio>
#include <cinttypes>
#include <functional>

namespace gl {
	class VBO;
	class VAO;
	
	namespace BasicMeshLoader {
		class Mesh;
	}
}

namespace qgl {
	
	class VertexAttributesConfiguration {
	public:
		
		VertexAttributesConfiguration(VertexAttributesConfiguration&&) = default;
		
		inline VertexAttributesConfiguration(
				void(*setupVAO)(gl::VAO* vao, gl::VBO* vertex, gl::VBO* elements),
				void(*copyData)(gl::BasicMeshLoader::Mesh* mesh, gl::VBO* vertex, gl::VBO* elements)) :
			setupVAO(setupVAO), copyData(copyData) {
		}
		
// 		inline VertexAttributesConfiguration(
// 				std::function<void(gl::VAO* vao, gl::VBO* vertex, gl::VBO* elements)> setupVAO,
// 				std::function<void(gl::BasicMeshLoader::Mesh* mesh, gl::VBO* vertex, gl::VBO* elements)> copyData) :
// 			setupVAO(setupVAO), copyData(copyData) {
// 		}
		
		inline void SetupVAO(gl::VAO* vao, gl::VBO* vertex, gl::VBO* elements) {
			setupVAO(vao, vertex, elements);
		}
		
		inline void CopyData(gl::BasicMeshLoader::Mesh* mesh, gl::VBO* vertex, gl::VBO* elements) {
			copyData(mesh, vertex, elements);
		}
		
		
		VertexAttributesConfiguration() = delete;
		VertexAttributesConfiguration(VertexAttributesConfiguration&) = delete;
		VertexAttributesConfiguration(const VertexAttributesConfiguration&) = delete;
		VertexAttributesConfiguration(VertexAttributesConfiguration*) = delete;
		VertexAttributesConfiguration(const VertexAttributesConfiguration*) = delete;
		
		inline bool operator==(const VertexAttributesConfiguration& other) const {
			return this == &other;
		}
		inline bool operator!=(const VertexAttributesConfiguration& other) const {
			return this != &other;
		}
		
		inline bool operator==(const VertexAttributesConfiguration* other) const {
			return this == other;
		}
		inline bool operator!=(const VertexAttributesConfiguration* other) const {
			return this != other;
		}
	
	protected:
		
		const std::function<void(gl::VAO* vao, gl::VBO* vertex, gl::VBO* elements)> setupVAO;
		const std::function<void(gl::BasicMeshLoader::Mesh* mesh, gl::VBO* vertex, gl::VBO* elements)> copyData;
	};
	
	namespace AttributeConfigDefaults {
		extern const VertexAttributesConfiguration ANIMATED_COLOR1_UV0_NORMAL;
// 		extern const VertexAttributesConfiguration ANIMATED_COLOR1_UV1_NORMAL;
// 		extern const VertexAttributesConfiguration ANIMATED_COLOR2_UV1_NORMAL;
// 		extern const VertexAttributesConfiguration ANIMATED_COLOR2_UV0_NORMAL;
// 		extern const VertexAttributesConfiguration ANIMATED_COLOR1_UV0;
// 		extern const VertexAttributesConfiguration ANIMATED_COLOR1_UV1;
// 		extern const VertexAttributesConfiguration ANIMATED_COLOR2_UV1;
// 		extern const VertexAttributesConfiguration ANIMATED_COLOR2_UV0;
// 		extern const VertexAttributesConfiguration STATIC_COLOR1_UV0_NORMAL;
// 		extern const VertexAttributesConfiguration STATIC_COLOR1_UV1_NORMAL;
// 		extern const VertexAttributesConfiguration STATIC_COLOR2_UV1_NORMAL;
// 		extern const VertexAttributesConfiguration STATIC_COLOR2_UV0_NORMAL;
// 		extern const VertexAttributesConfiguration STATIC_COLOR1_UV0;
// 		extern const VertexAttributesConfiguration STATIC_COLOR1_UV1;
// 		extern const VertexAttributesConfiguration STATIC_COLOR2_UV1;
// 		extern const VertexAttributesConfiguration STATIC_COLOR2_UV0;
	}
}

#endif

