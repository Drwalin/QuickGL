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

#include "util/AllocatorVBO.hpp"
#include "util/BufferedVBO.hpp"
#include "util/IdsManager.hpp"

namespace gl {
	class VBO;
	namespace BasicMeshLoader {
		class Mesh;
	}
}

namespace qgl {
	
	class MeshManager {
	public:
		
		struct MeshInfo {
			MeshInfo() = default;
			std::string name;
			uint32_t firstElement;
			uint32_t countElements;
			uint32_t firstVertex;
			uint32_t countVertices;
		};
		
		MeshManager(uint32_t vertexSize,
				void(*meshAppenderVertices)(
					std::vector<uint8_t>& buffer,
					uint32_t bufferByteOffset,
					gl::BasicMeshLoader::Mesh* mesh
				));
		~MeshManager();
		
		bool LoadModels(const std::string& fileName);
		
		MeshInfo GetMeshInfoById(uint32_t id) const;
		uint32_t GetMeshIdByName(std::string name) const;
		
		uint32_t CreateMeshFrom(std::shared_ptr<MeshManager> otherMeshManager,
				const std::vector<uint32_t>& sourceMeshesIds);
		
		void ReleaseMeshReference(uint32_t id);
		
		void GetMeshIndices(uint32_t meshId, uint32_t& indexStart,
				uint32_t& indexCount);
		

		std::shared_ptr<gl::VBO> GetVBO() { return vbo; }
		std::shared_ptr<gl::VBO> GetEBO() { return ebo; }
		
	protected:
		
		void FreeMesh(uint32_t id);
		
	protected:
		
		std::map<std::string, uint32_t> mapNameToId;
		std::vector<MeshInfo> meshInfo;
		IdsManager idsManager;
		
		AllocatorVBO vboAllocator;
		std::shared_ptr<gl::VBO> vbo;
		
		AllocatorVBO eboAllocator;
		std::shared_ptr<gl::VBO> ebo;
		
		void(*const meshAppenderVertices)(
				std::vector<uint8_t>& buffer,
				uint32_t bufferByteOffset,
				gl::BasicMeshLoader::Mesh* mesh
				);
		
		const uint32_t vertexSize;
	};
}

#endif

