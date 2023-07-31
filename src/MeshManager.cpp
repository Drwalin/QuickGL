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
	MeshManager::MeshManager(std::shared_ptr<Engine> engine, uint32_t vertexSize,
			bool(*meshAppenderVertices)(
				std::vector<uint8_t>& buffer,
				uint32_t bufferByteOffset,
				gl::BasicMeshLoader::Mesh* mesh))
		: perMeshInfoGPU(engine),
			vboAllocator(vertexSize, false), vbo(vboAllocator.Vbo()),
			eboAllocator(sizeof(uint32_t), true), ebo(eboAllocator.Vbo()),
			meshAppenderVertices(meshAppenderVertices),
			vertexSize(vertexSize) {
		perMeshInfoGPU.Init();
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
		for(auto& mesh : loader->meshes) {
			LoadMesh(mesh.get());
		}
		return loader->meshes.size() > 0;
	}
	
	uint32_t MeshManager::CreateMeshFromData(std::string name,
			const std::vector<glm::vec3>& pos,
			const std::vector<glm::vec3>& normal,
			const std::vector<std::vector<glm::vec4>>& color,
			const std::vector<std::vector<glm::vec2>>& uv,
			const std::vector<uint32_t>& indices,
			float boundingSphereRadiusMultiplier) {
		gl::BasicMeshLoader::Mesh mesh;
		
		mesh.name = name;
		
		mesh.pos.insert(mesh.pos.begin(), pos.begin(), pos.end());
		mesh.normal.insert(mesh.normal.begin(), normal.begin(), normal.end());
		mesh.uv.resize(uv.size());
		for(int i=0; i<uv.size(); ++i)
			mesh.uv[i].insert(mesh.uv[i].begin(), uv[i].begin(), uv[i].end());
		mesh.color.resize(color.size());
		for(int i=0; i<color.size(); ++i)
			mesh.color[i].insert(mesh.color[i].begin(), color[i].begin(), color[i].end());
		mesh.indices = indices;
		
		mesh.boundingBoxMax = mesh.boundingBoxMin = pos[0];
		for(glm::vec3 v : pos) {
			mesh.boundingBoxMin.x = std::min(mesh.boundingBoxMin.x, v.x);
			mesh.boundingBoxMin.y = std::min(mesh.boundingBoxMin.y, v.y);
			mesh.boundingBoxMin.z = std::min(mesh.boundingBoxMin.z, v.z);
			mesh.boundingBoxMax.x = std::max(mesh.boundingBoxMax.x, v.x);
			mesh.boundingBoxMax.y = std::max(mesh.boundingBoxMax.y, v.y);
			mesh.boundingBoxMax.z = std::max(mesh.boundingBoxMax.z, v.z);
		}
		
		mesh.boundingSphereCenter =
			(mesh.boundingBoxMin + mesh.boundingBoxMax) * 0.5f;
		float r2 = 0;
		for(glm::vec3 v : pos) {
			r2 = std::max(r2, glm::dot(v-mesh.boundingSphereCenter,
						v-mesh.boundingSphereCenter));
		}
		mesh.boundingSphereRadius = sqrt(r2)*boundingSphereRadiusMultiplier;
		
		LoadMesh(&mesh);
		
		return GetMeshIdByName(name);
	}
	
	bool MeshManager::LoadMesh(gl::BasicMeshLoader::Mesh* mesh) {
		std::vector<uint8_t> vboSrc, eboSrc;
		if(meshAppenderVertices(vboSrc, 0, mesh)) {
			PerMeshInfoGPU info;
			mesh->GetBoundingSphereInfo(info.boundingSphereCenterOffset,
				info.boundingSphereRadius);
			
			info.countVertices = mesh->pos.size();
			info.firstVertex = vboAllocator.Allocate(info.countVertices);
			
			eboSrc.clear();
			mesh->AppendIndices<uint32_t>(info.firstVertex, eboSrc);
			info.countElements = mesh->indices.size();
			info.firstElement = eboAllocator.Allocate(info.countElements);
			
			std::string name = mesh->name;
			uint32_t meshId = idsManager.GetNewId();
			mapNameToId[name] = meshId;
			perMeshInfoGPU.SetValue(info, meshId);
			if(perMeshInfoCPU.size() <= meshId) {
				perMeshInfoCPU.resize(meshId+100);
			}
			perMeshInfoCPU[meshId] = {nullptr, name};
			
			vbo.Update(&vboSrc.front(), info.firstVertex*vertexSize,
					info.countVertices*vertexSize);
			
			ebo.Update(&eboSrc.front(), info.firstElement*sizeof(uint32_t),
					info.countElements*sizeof(uint32_t));
			return true;
		}
		return false;
	}
	
	void MeshManager::UpdateVbo() {
		perMeshInfoGPU.UpdateVBO();
	}
	
	MeshManager::PerMeshInfoGPU MeshManager::GetMeshInfoById(uint32_t id) const {
		return perMeshInfoGPU.GetValue(id);
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
		auto info = GetMeshInfoById(meshId);
		indexStart = info.firstElement;
		indexCount = info.countElements;
	}
	
	void MeshManager::GetMeshBoundingSphere(uint32_t meshId, float* offset,
			float& radius) {
		auto info = GetMeshInfoById(meshId);
		memcpy(offset, info.boundingSphereCenterOffset, sizeof(float)*3);
		radius = info.boundingSphereRadius;
	}
	
	void MeshManager::FreeMesh(uint32_t id) {
		auto info = GetMeshInfoById(id);
		vboAllocator.Free(info.firstVertex, info.countVertices);
		eboAllocator.Free(info.firstElement, info.countElements);
	}
	
	void MeshManager::AcquireMeshReference(uint32_t id) {
// 		throw "Meshmanager::AcquireMeshReference is not implemented.";
	}
	
	void MeshManager::ReleaseMeshReference(uint32_t id) {
// 		throw "Meshmanager::ReleaseMeshReference is not implemented.";
	}
}

