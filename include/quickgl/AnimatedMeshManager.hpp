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
		
		gl::Texture& GetAnimationsMetadataTexture() { return *metaInfo; }
		gl::Texture& GetKeyframesTexture() { return *matrices; }
		
		virtual void ReleaseMeshReference(uint32_t id) override;
		
		friend class PipelineAnimated;
		
	protected:
		
		virtual void FreeMesh(uint32_t id) override;
		
		virtual bool LoadModels(
				std::shared_ptr<gl::BasicMeshLoader::AssimpLoader> loader)
			override;
		
	private:
		
		struct AnimationInfo {
			uint32_t firstMatrixId;
			uint32_t bonesCount;
			uint32_t framesCount;
			uint32_t fps;
		};
		
		std::map<std::string, uint32_t> mapAnimationNameToId;
		std::vector<AnimationInfo> animationsInfo;
		
		
		std::vector<glm::mat4> matricesHost;
		
		// TEXTURE_1D 16384 animations max
		std::shared_ptr<gl::Texture> metaInfo; // RGBA32UI, single texel:
											   // R - id of first matrix
											   // G - number of bones
											   // B - number of key frames
											   // A - number of FPS
		
		// TEXTURE_2D_ARRAY 16384*64 x LAYERS; 16 bones of single frame in
		//                                     single row
		std::shared_ptr<gl::Texture> matrices; // RGBA32F, 4 consecutive
											   // vertical texels make up a
											   // single matrix.
											   // Bones ids are going:
											   //  1. from top to down
											   //  2. from left to right
	};
}

#endif

