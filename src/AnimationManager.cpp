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
	}
	
	void AnimationManager::LoadAnimations(
			std::shared_ptr<gl::BasicMeshLoader::AssimpLoader> loader) {
		uint32_t firstToUpdate = metaInfo.Count();
		metaInfo.Resize(firstToUpdate + loader->animations.size());
		for(auto& anim : loader->animations) {
			uint32_t animationId = metaInfo.Count();
			mapAnimationNameToId[anim->name] = animationId;
			AnimationInfo info;
			info.firstMatrixId = matricesHost.size();
			info.fps = 24;
			info.bonesCount = anim->CountBones();
			info.framesCount = anim->duration * anim->framesPerSecond;
			metaInfo[animationId] = info;
			for(uint32_t i=0; i<info.framesCount; ++i) {
				uint32_t offset = matricesHost.size();
				matricesHost.resize(offset + info.bonesCount);
				anim->GetModelBoneMatrices(&(matricesHost[offset]),
						i/(float)info.fps, false);
			}
		}
		printf(" matrices size: %llu\n", matricesHost.size());
		matricesHost.reserve(matricesHost.size() + 16384*4*64);
		for(int i=0; i<matricesHost.size(); ++i) {
// 			matricesHost[i] = glm::mat4(1);
		}
		uint32_t w, h, d;
		w = 64;
		h = (matricesHost.size()*4+64-1)/64;
		d = (((matricesHost.size()*4+64-1)/64)+16384-1)/(16384);
		printf(" whd: %u %u %u\n", w, h, d);
		matrices->Update3(matricesHost.data(),
				0, 0, 0,
				w, h, d,
				0,
				gl::TextureDataFormat::RGBA, gl::DataType::FLOAT);
		metaInfo.UpdateVertices(firstToUpdate, loader->animations.size());
		std::vector<glm::mat4> mats;
		mats.resize(w*h*d);
		matrices->Fetch3(mats.data(), 0, 0, 0, w, h, d, 0, gl::TextureDataFormat::RGBA, gl::DataType::FLOAT, w*h*d*64);
		
		
// 		float *a = (float*)matricesHost.data();
// 		float *b = (float*)mats.data();
// 		for(int i = 0; i<16*10; ++i) {
// 			printf(" bone %i : %f == %f\n", i/16, a[i], b[i]);
// 			if(i%16 == 15)
// 				printf("\n");
// 		}
	}
}

