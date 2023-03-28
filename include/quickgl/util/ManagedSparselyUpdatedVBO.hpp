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

#ifndef QUICKGL_MANAGED_SPARSELY_UPDATED_VBO_HPP
#define QUICKGL_MANAGED_SPARSELY_UPDATED_VBO_HPP

#include <cinttypes>

#include <vector>
#include <unordered_map>

namespace gl {
	class Shader;
	class VBO;
}

namespace qgl {
	class UntypedManagedSparselyUpdatedVBO {
	public:
		
		UntypedManagedSparselyUpdatedVBO(uint32_t elementSize);
		~UntypedManagedSparselyUpdatedVBO();
		
		void Init();
		void Destroy();
		
		void Resize(uint32_t size);
		
		inline gl::VBO& Vbo() { return *vbo; }
		
		void UpdateVBO();
		
		void SetValue(const void* value, uint32_t id);
		
		uint32_t Count() const;
		
	private:
		
		gl::VBO* vbo;
		gl::VBO* deltaVbo;
		gl::Shader* shader;
		
		uint32_t maxId;
		int32_t shaderDeltaCommandsLocation;
		
		std::vector<uint8_t> deltaData;
		std::unordered_map<uint32_t, uint32_t> whereSomethingWasUpdated;
		
		const uint32_t ELEMENT_SIZE;
		const uint32_t UPDATE_STRUCUTRE_SIZE;
	};
	
	
	
	template<typename T>
	class ManagedSparselyUpdatedVBO final : public UntypedManagedSparselyUpdatedVBO {
	public:
		
		ManagedSparselyUpdatedVBO() : UntypedManagedSparselyUpdatedVBO(sizeof(T)) {}
		~ManagedSparselyUpdatedVBO() {}
		
		void SetValue(const T& value, uint32_t id) {
			UntypedManagedSparselyUpdatedVBO::SetValue((const void*)&value, id);
		}
	};
}

#endif

