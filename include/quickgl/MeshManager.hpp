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

#ifndef QUICKGL_MESH_MANAGER_HPP
#define QUICKGL_MESH_MANAGER_HPP

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>

#include <glm/glm.hpp>

#include "util/AllocatorVBO.hpp"
#include "util/BufferedVBO.hpp"
#include "util/IdsManager.hpp"
#include "util/Log.hpp"
#include "util/ManagedSparselyUpdatedVBO.hpp"

namespace gl {
	class VBO;
	namespace BasicMeshLoader {
		class Mesh;
		class AssimpLoader;
	}
}

namespace qgl {
	class MeshResource;
	class Engine;
	
	class MeshManager {
	public:
		
		struct PerMeshInfoGPU {
			uint32_t firstVertex;
			uint32_t countVertices;
			uint32_t firstElement;
			uint32_t countElements;
			float boundingSphereCenterOffset[3];
			float boundingSphereRadius;
		};
		
		MeshManager(std::shared_ptr<Engine> engine, uint32_t vertexSize,
				bool(*meshAppenderVertices)(
					std::vector<uint8_t>& buffer,
					uint32_t bufferByteOffset,
					gl::BasicMeshLoader::Mesh* mesh
				));
		virtual ~MeshManager();
		
		bool LoadModels(const std::string& fileName);
		
		PerMeshInfoGPU GetMeshInfoById(uint32_t id) const;
		uint32_t GetMeshIdByName(std::string name) const;
		
		uint32_t CreateMeshFrom(std::shared_ptr<MeshManager> otherMeshManager,
				const std::vector<uint32_t>& sourceMeshesIds);
		
		virtual void AcquireMeshReference(uint32_t id);
		virtual void ReleaseMeshReference(uint32_t id);
		
		void GetMeshIndices(uint32_t meshId, uint32_t& indexStart,
				uint32_t& indexCount);
		void GetMeshBoundingSphere(uint32_t meshId, float* offset,
				float& radius);
		
		uint32_t CreateMeshFromData(std::string name,
				const std::vector<glm::vec3>& pos,
				const std::vector<glm::vec3>& normal,
				const std::vector<std::vector<glm::vec4>>& color,
				const std::vector<std::vector<glm::vec2>>& uv,
				const std::vector<uint32_t>& indices,
				float boundingSphereRadiusMultiplier=1.0f);
		
		bool LoadMesh(gl::BasicMeshLoader::Mesh* mesh);
		
		gl::VBO& GetVBO() { return vbo; }
		gl::VBO& GetEBO() { return ebo; }
		
		gl::VBO& GetMeshInfoBuffer() { return perMeshInfoGPU.Vbo(); }
		
		void UpdateVbo();
		
	protected:
		
		virtual void FreeMesh(uint32_t id);
		virtual bool LoadModels(
				std::shared_ptr<gl::BasicMeshLoader::AssimpLoader> loader);
		
	protected:
		
		struct PerMeshInfoCPU {
			std::shared_ptr<MeshResource> meshResource;
			std::string name;
		};
		
		ManagedSparselyUpdatedVBOWithLocal<PerMeshInfoGPU> perMeshInfoGPU;
		std::vector<PerMeshInfoCPU> perMeshInfoCPU;
		
		std::map<std::string, uint32_t> mapNameToId;
		IdsManager idsManager;
		
		AllocatorVBO vboAllocator;
		gl::VBO& vbo;
		
		AllocatorVBO eboAllocator;
		gl::VBO& ebo;
		
		bool(*const meshAppenderVertices)(
				std::vector<uint8_t>& buffer,
				uint32_t bufferByteOffset,
				gl::BasicMeshLoader::Mesh* mesh
				);
		
		const uint32_t vertexSize;
	};
}

#endif

