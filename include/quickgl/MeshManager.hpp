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

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VAO.hpp"

#include "AllocatorVBO.hpp"

namespace qgl {
	
	class MeshManager {
	public:
		
		struct MeshInfo {
			std::string name;
			uint32_t firstElement;
			uint32_t countElements;
			uint32_t firstVertex;
			uint32_t countVertices;
		};
		
		MeshManager();
		~MeshManager();
		
		MeshInfo& GetMeshInfoById(uint32_t id);
		uint32_t GetMeshIdByName(std::string name);
		
		uint32_t CreateMeshFrom(MeshManager* otherMeshManager,
				const std::vector<uint32_t>& sourceMeshesIds);
		
		void FreeMesh(uint32_t id);
		
	private:
		
		std::map<std::string, uint32_t> mapNameToId;
		std::vector<MeshInfo> meshInfo;
		std::set<uint32_t> freeMeshIds;
		
		AllocatorVBO verticesBuffer;
		AllocatorVBO elementsBuffer;
		std::shared_ptr<gl::VAO> vao;
	};
}

#endif

