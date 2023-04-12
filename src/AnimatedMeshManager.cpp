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

#include "../include/quickgl/AnimatedMeshManager.hpp"

namespace qgl {
	AnimatedMeshManager::AnimatedMeshManager(uint32_t vertexSize,
			void(*meshAppenderVertices)(
				std::vector<uint8_t>& buffer,
				uint32_t bufferByteOffset,
				gl::BasicMeshLoader::Mesh* mesh)) :
	MeshManager(vertexSize, meshAppenderVertices) {
		metaInfo = std::make_shared<gl::Texture>();
		metaInfo->Generate1(gl::TextureTarget::TEXTURE_1D, 4096,
				gl::TextureSizedInternalFormat::RGBA32UI,
				gl::TextureDataFormat::RGBA_INTEGER, gl::DataType::UNSIGNED_INT);
		matrices = std::make_shared<gl::Texture>();
		matrices->Generate3(gl::TextureTarget::TEXTURE_2D_ARRAY, 64, 16384, 1,
				gl::TextureSizedInternalFormat::RGBA32F,
				gl::TextureDataFormat::RGBA, gl::DataType::FLOAT);
	}
	
	AnimatedMeshManager::~AnimatedMeshManager() {
	}
	
	
	void AnimatedMeshManager::ReleaseMeshReference(uint32_t id) {
		MeshManager::ReleaseMeshReference(id);
		throw "AnimatedMeshManager::ReleaseMeshReference is not implemented.";
	}
	
	void AnimatedMeshManager::FreeMesh(uint32_t id) {
		MeshManager::FreeMesh(id);
		throw "AnimatedMeshManager::FreeMesh is not implemented.";
	}
	
	
	bool AnimatedMeshManager::LoadModels(
			std::shared_ptr<gl::BasicMeshLoader::AssimpLoader> loader) {
		bool ret = MeshManager::LoadModels(loader);
		struct FullAnimationMetaData {
			uint32_t frames;
			uint32_t bones;
			uint32_t animationId;
			uint32_t fps;
			std::string name;
			uint32_t firstBoneInBuffer;
		};
		
		for(auto& anim : loader->animations) {
			uint32_t animationId = animationsInfo.size();
			mapAnimationNameToId[anim->name] = animationId;
			AnimationInfo info;
			info.firstMatrixId = matricesHost.size();
			info.fps = 24;
			info.bonesCount = anim->CountBones();
			info.framesCount = anim->duration * anim->framesPerSecond;
			
			for(uint32_t i=0; i<info.framesCount; ++i) {
				matricesHost.resize(matricesHost.size()
						+info.bonesCount);
				anim->GetModelBoneMatrices(&(matricesHost[
							matricesHost.size()-info.bonesCount]),
						i/(float)info.fps, false);
			}
			animationsInfo.emplace_back(info);
		}
		matricesHost.reserve(matricesHost.size() + 1024*128);
		matrices->Update3(&matricesHost.front(),
				0, 0, 0,
				64, (matricesHost.size()*4+63)/64, (matricesHost.size()*4+64*16384-1)/(64*16384),
				0,
				gl::TextureDataFormat::RGBA, gl::DataType::FLOAT);
		metaInfo->Update1(&matricesHost.front(), 0,
				matricesHost.size()*4, 0,
				gl::TextureDataFormat::RGBA, gl::DataType::UNSIGNED_INT);
		
		return ret | (loader->animations.size() != 0);
	}
}

