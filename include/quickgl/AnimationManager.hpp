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

#ifndef QUICKGL_ANIMATION_MANAGER_HPP
#define QUICKGL_ANIMATION_MANAGER_HPP

#include <memory>
#include <vector>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "util/BufferedVBO.hpp"

namespace gl {
	class Texture;
	class Shader;
	class VBO;
	namespace BasicMeshLoader {
		class AssimpLoader;
	}
}

namespace qgl {
	class AnimationManager {
	public:
		
		struct AnimationInfo {
			uint32_t firstMatrixId;
			uint32_t bonesCount;
			uint32_t framesCount;
			uint32_t fps;
		};
		
		AnimationManager();
		~AnimationManager();
		
		inline TypedVBO<AnimationInfo>& GetAnimationsMetadata() { return metaInfo; }
		inline gl::Texture& GetKeyframesTexture() { return *matrices; }
		
		uint32_t GetAnimationId(const std::string& animationName);
		
		friend class AnimatedMeshManager;
		
	private:
		
		void LoadAnimations(
				std::shared_ptr<gl::BasicMeshLoader::AssimpLoader> loader);
		
		void UpdateVRAM();
		
	private:
		
		std::unordered_map<std::string, uint32_t> mapAnimationNameToId;
		
		
		std::vector<glm::mat4> matricesHost;
		
		// TEXTURE_1D 16384 animations max
		TypedVBO<AnimationInfo> metaInfo;
		
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

