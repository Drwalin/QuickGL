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

#include <thread>

#include "../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../OpenGLWrapper/include/openglwrapper/Shader.hpp"
#include "../OpenGLWrapper/include/openglwrapper/Texture.hpp"
#include "../OpenGLWrapper/include/openglwrapper/basic_mesh_loader/AssimpLoader.hpp"

#include "../include/quickgl/AnimationManager.hpp"

namespace qgl {
	AnimationManager::AnimationManager() {
		matrices = std::make_shared<gl::Texture>();
		matrices->Generate3(gl::TextureTarget::TEXTURE_2D_ARRAY, 64, 16384, 16,
				gl::TextureSizedInternalFormat::RGBA32F,
				gl::TextureDataFormat::RGBA, gl::DataType::FLOAT);
	}
	
	AnimationManager::~AnimationManager() {
		matrices->Destroy();
		matrices = nullptr;
		metaData.Destroy();
	}
	
	void AnimationManager::LoadAnimations(
			std::shared_ptr<gl::BasicMeshLoader::AssimpLoader> loader) {
		const uint32_t firstToUpdate = mapAnimationNameToId.size();
		metaData.Resize(firstToUpdate + loader->animations.size());
		uint32_t animationId = firstToUpdate;
		for(auto& anim : loader->animations) {
			mapAnimationNameToId[anim->name] = animationId;
			printf("Animation: '%s' -> %i\n", anim->name.c_str(), animationId);
			AnimationInfo info;
			info.firstMatrixId = matricesHost.size();
			info.fps = 24;
			info.bonesCount = anim->CountBones();
			info.framesCount = anim->duration * anim->framesPerSecond;
			printf("Animation frames: %i, bones: %i\n", info.framesCount, info.bonesCount);
			printf(" first matrix = %i\n", info.firstMatrixId);
			metaData[animationId] = info;
			for(uint32_t i=0; i<info.framesCount; ++i) {
				uint32_t offset = matricesHost.size();
				matricesHost.resize(offset + info.bonesCount);
				anim->GetModelBoneMatrices(&(matricesHost[offset]),
						i/(float)info.fps, false);
			}
			++animationId;
		}
		
		if(firstToUpdate == metaData.Count()) {
			return;
		}
		
		matricesHost.reserve(matricesHost.size() + 16384*4*64);
		
		uint32_t w, h, d;
		w = 64;
		h = (matricesHost.size()*4+64-1)/64;
		d = (((matricesHost.size()*4+64-1)/64)+16384-1)/(16384);
		matrices->Update3(matricesHost.data(),
				0, 0, 0,
				w, h, d,
				0,
				gl::TextureDataFormat::RGBA, gl::DataType::FLOAT);
		metaData.UpdateVertices(firstToUpdate, loader->animations.size());
		metaData.UpdateVertices(0, metaData.Count());
		printf(" meta data count = %i\n", metaData.Count());
	}
}

