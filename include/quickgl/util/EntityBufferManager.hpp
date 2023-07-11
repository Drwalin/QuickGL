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

#ifndef QUICKGL_ENTITY_BUFFER_MANAGER_HPP
#define QUICKGL_ENTITY_BUFFER_MANAGER_HPP

#include <cinttypes>

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <memory>

#include "../../include/quickgl/util/ManagedSparselyUpdatedVBO.hpp"

namespace gl {
	class VBO;
}

namespace qgl {
	class EntityBufferManager final {
	public:
		
		struct PairMove {
			uint32_t from;
			uint32_t to;
		};
		
		struct BufferInfo {
			void (*reserve)(uint32_t newCapacity);
			void (*resize)(uint32_t newSize);
			void (*moveByVbo)(gl::VBO* deltaVbo);
			void (*moveByOne)(uint32_t from, uint32_t to);
			void (*updateVbo)(uint32_t stageId);
			
			void* data;
		};
		
		EntityBufferManager();
		~EntityBufferManager();
		
		void Init();
		void Destroy();
		
		uint32_t GetNewEntity();
		void FreeEntity(uint32_t entity);
		
		void UpdateBuffers(uint32_t stageId);
		
	public:
		
		void AddVBO(gl::VBO* vbo);
		template<typename T>
		void AddVector(std::vector<T>* vec);
		void AddManagedSparselyUpdateVBO(qgl::UntypedManagedSparselyUpdatedVBO* vbo);
		
		uint32_t GetOffsetOfEntity(uint32_t entity) const;
		
	private:
		
		void GenerateDeltaBuffer();
		
	private:
		
		std::unordered_map<uint32_t, uint32_t> deltaFromTo;
		std::unordered_map<uint32_t, uint32_t> deltaToFrom;
		gl::VBO* deltaVbo;
		std::vector<PairMove> deltaBuffer;
		std::vector<uint32_t> freeingEntites;
		
		ManagedSparselyUpdatedVBOWithLocal<uint32_t> mapOffsetToEntity;
		std::unordered_map<uint32_t, uint32_t> mapEntityToOffset;
		
		std::vector<BufferInfo> buffers;
		
		uint32_t lastAddedEntity;
		uint32_t entitiesBufferSize;
	};
	
	template<typename T>
	void EntityBufferManager::AddVector(std::vector<T>* vec) {
		buffers.emplace_back(
			[vec](uint32_t capacity) {
				vec->reserve(capacity);
			},
			[vec](uint32_t size) {
				vec->resize(size);
			},
			nullptr,
			[vec](uint32_t from, uint32_t to) {
				std::swap(vec[0][from], vec[0][to]);
			},
			nullptr,
			vec
		);
	}
}

#endif

