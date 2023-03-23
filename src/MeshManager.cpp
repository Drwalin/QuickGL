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
	
	bool MeshManager::LoadModels(const std::string& fileName) {
		gl::BasicMeshLoader::AssimpLoader l;
		if(l.Load(fileName) == false)
			return false;
		std::vector<uint8_t> vboSrc, eboSrc;
		for(auto mesh : l.meshes) {
			vboSrc.clear();
			eboSrc.clear();
			meshAppenderVertices(vboSrc, 0, mesh.get());
			MeshInfo info;
			info.countElements = mesh->pos.size();
			info.countVertices = mesh->indices.size();
			info.firstVertex = vboAllocator.Allocate(mesh->pos.size());
			info.firstElement = eboAllocator.Allocate(mesh->indices.size());
			
			mesh->AppendIndices<uint32_t>(info.firstVertex, eboSrc);
			
			std::string name = mesh->name;
			uint32_t meshId = idsManager.GetNewId();
			mapNameToId[name] = meshId;
			if(meshInfo.size() <= meshId) {
				meshInfo.resize(meshId+100);
			}
			meshInfo[meshId] = info;
			if(vbo.GetVertexCount() < info.firstVertex+info.countVertices)
				vbo.Resize(info.firstVertex+info.countVertices);
			
			if(ebo.GetVertexCount() < info.firstElement+info.countElements)
				ebo.Resize(info.firstElement+info.countElements);
			vbo.Update(&vboSrc.front(), info.firstVertex*vertexSize,
					info.countVertices*vertexSize);
			ebo.Update(&eboSrc.front(), info.firstElement*sizeof(uint32_t),
					info.countElements*sizeof(uint32_t));
		}
	GL_CHECK_PUSH_ERROR;
		return l.meshes.size() > 0;
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
}

