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

#ifndef QUICKGL_ANIMATED_MESH_MANAGER_HPP
#define QUICKGL_ANIMATED_MESH_MANAGER_HPP

#include <memory>
#include <vector>

#include "InputManager.hpp"
#include "util/RenderStageComposer.hpp"
#include "AnimationManager.hpp"

#include "MeshManager.hpp"

namespace gl {
	class Texture;
	class Shader;
	class VBO;
}

namespace qgl {
	class AnimatedMeshManager : public MeshManager {
	public:
		
		AnimatedMeshManager(uint32_t vertexSize,
				void(*meshAppenderVertices)(
					std::vector<uint8_t>& buffer,
					uint32_t bufferByteOffset,
					gl::BasicMeshLoader::Mesh* mesh
				));
		virtual ~AnimatedMeshManager();
		
		virtual void ReleaseMeshReference(uint32_t id) override;
		
		AnimationManager& GetAnimationManager() { return *animationManager; }
		
		friend class PipelineAnimated;
		friend class AnimationManager;
		
	protected:
		
		virtual void FreeMesh(uint32_t id) override;
		
		virtual bool LoadModels(
				std::shared_ptr<gl::BasicMeshLoader::AssimpLoader> loader)
			override;
		
	private:
		
		AnimationManager* animationManager;
	};
}

#endif

