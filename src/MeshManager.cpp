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

#include <memory>
#include <vector>
#include <map>

#include "../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../OpenGLWrapper/include/openglwrapper/VAO.hpp"
#include "../OpenGLWrapper/include/openglwrapper/Texture.hpp"
#include "../OpenGLWrapper/include/openglwrapper/basic_mesh_loader/AssimpLoader.hpp"

#include "../include/quickgl/MeshManager.hpp"

namespace qgl {
	MeshManager::MeshManager(uint32_t vertexSize,
			void(*meshAppenderVertices)(
				std::vector<uint8_t>& buffer,
				uint32_t bufferByteOffset,
				gl::BasicMeshLoader::Mesh* mesh))
		: vboAllocator(vertexSize, false), vbo(vboAllocator.Vbo()),
			eboAllocator(sizeof(uint32_t), true), ebo(eboAllocator.Vbo()),
			meshAppenderVertices(meshAppenderVertices),
			vertexSize(vertexSize) {
	}
	
	MeshManager::~MeshManager() {
	}
	
	bool MeshManager::LoadModels(
			const std::string& fileName) {
		std::shared_ptr<gl::BasicMeshLoader::AssimpLoader> loader
			= std::make_shared<gl::BasicMeshLoader::AssimpLoader>();
		if(loader->Load(fileName) == false)
			return false;
		if(loader->meshes.size() == 0)
			return false;
		return LoadModels(loader);
	}
	
	bool MeshManager::LoadModels(std::shared_ptr<gl::BasicMeshLoader::AssimpLoader> loader) {
		std::vector<uint8_t> vboSrc, eboSrc;
		for(auto& mesh : loader->meshes) {
			MeshInfo info;
			
			mesh->GetBoundingSphereInfo(info.boundingSphereCenterOffset,
					info.boundingSphereRadius);
			
			vboSrc.clear();
			meshAppenderVertices(vboSrc, 0, mesh.get());
			info.countVertices = mesh->pos.size();
			info.firstVertex = vboAllocator.Allocate(info.countVertices);
			
			eboSrc.clear();
			mesh->AppendIndices<uint32_t>(info.firstVertex, eboSrc);
			info.countElements = mesh->indices.size();
			info.firstElement = eboAllocator.Allocate(info.countElements);
			
			std::string name = mesh->name;
			uint32_t meshId = idsManager.GetNewId();
			mapNameToId[name] = meshId;
			if(meshInfo.size() <= meshId) {
				meshInfo.resize(meshId+100);
			}
			meshInfo[meshId] = info;
			
			vbo.Update(&vboSrc.front(), info.firstVertex*vertexSize,
					info.countVertices*vertexSize);
			
			ebo.Update(&eboSrc.front(), info.firstElement*sizeof(uint32_t),
					info.countElements*sizeof(uint32_t));
		}
		return loader->meshes.size() > 0;
	}
	
	MeshManager::MeshInfo MeshManager::GetMeshInfoById(uint32_t id) const {
		return meshInfo[id];
	}
	
	uint32_t MeshManager::GetMeshIdByName(std::string name) const {
		auto it = mapNameToId.find(name);
		if(it != mapNameToId.end()) {
			return it->second;
		}
		return 0;
	}
	
	void MeshManager::GetMeshIndices(uint32_t meshId, uint32_t& indexStart,
			uint32_t& indexCount) {
		MeshInfo info = GetMeshInfoById(meshId);
		indexStart = info.firstElement;
		indexCount = info.countElements;
	}
	
	void MeshManager::GetMeshBoundingSphere(uint32_t meshId, float* offset,
			float& radius) {
		MeshInfo info = GetMeshInfoById(meshId);
		memcpy(offset, info.boundingSphereCenterOffset, sizeof(float)*3);
		radius = info.boundingSphereRadius;
	}
	
	void MeshManager::FreeMesh(uint32_t id) {
		throw "Meshmanager::FreeMesh is not implemented.";
	}
	
	void MeshManager::ReleaseMeshReference(uint32_t id) {
		throw "Meshmanager::ReleaseMeshReference is not implemented.";
	}
}

