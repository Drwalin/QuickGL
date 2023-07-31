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
	AnimatedMeshManager::AnimatedMeshManager(std::shared_ptr<Engine> engine,
			uint32_t vertexSize,
			bool(*meshAppenderVertices)(
				std::vector<uint8_t>& buffer,
				uint32_t bufferByteOffset,
				gl::BasicMeshLoader::Mesh* mesh)) :
			MeshManager(engine, vertexSize, meshAppenderVertices) {
		animationManager = new AnimationManager();
	}
	
	AnimatedMeshManager::~AnimatedMeshManager() {
		if(animationManager) {
			delete animationManager;
		}
		animationManager = nullptr;
	}
	
	void AnimatedMeshManager::ReleaseMeshReference(uint32_t id) {
		MeshManager::ReleaseMeshReference(id);
// 		throw "AnimatedMeshManager::ReleaseMeshReference is not implemented.";
	}
	
	void AnimatedMeshManager::FreeMesh(uint32_t id) {
		MeshManager::FreeMesh(id);
// 		throw "AnimatedMeshManager::FreeMesh is not implemented.";
	}
	
	bool AnimatedMeshManager::LoadModels(
			std::shared_ptr<gl::BasicMeshLoader::AssimpLoader> loader) {
		bool ret = MeshManager::LoadModels(loader);
		animationManager->LoadAnimations(loader);
		return ret;
	}
}

