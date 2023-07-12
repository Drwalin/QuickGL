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
			void (*reserve)(void* object, uint32_t newCapacity);
			void (*resize)(void* object, uint32_t newSize);
			void (*moveByVbo)(void* object, gl::VBO* deltaVbo);
			void (*moveByOne)(void* object, uint32_t from, uint32_t to);
			void (*updateVbo)(void* object, uint32_t stageId);
			
			void* data;
			void* funcData;
		};
		
		EntityBufferManager();
		~EntityBufferManager();
		
		void Init();
		void Destroy();
		
		uint32_t GetNewEntity();
		void FreeEntity(uint32_t entity);
		
		uint32_t Count() const;
		
		uint32_t UpdateBuffers(uint32_t stageId);
		
	public:
		
		void AddVBO(gl::VBO* vbo);
		template<typename T>
		void AddVector(std::vector<T>* vec);
		void AddManagedSparselyUpdateVBO(
				qgl::UntypedManagedSparselyUpdatedVBO* vbo);
		
		uint32_t GetOffsetOfEntity(uint32_t entity) const;
		
		static inline uint64_t GetAllEntitiesAdded() { return allEntitiesAdded; }
		
	private:
		
		void GenerateDeltaBuffer();
		
	private:
		
// 		std::unordered_map<uint32_t, uint32_t> deltaFromTo;
// 		std::unordered_map<uint32_t, uint32_t> deltaToFrom;
// 		gl::VBO* deltaVbo;
		std::vector<PairMove> deltaBuffer;
		std::vector<uint32_t> freeingEntites;
		
		ManagedSparselyUpdatedVBOWithLocal<uint32_t> mapOffsetToEntity;
		std::unordered_map<uint32_t, uint32_t> mapEntityToOffset;
		
		std::vector<BufferInfo> buffers;
		
		uint32_t lastAddedEntity;
		uint32_t entitiesBufferSize;
		uint32_t entitiesCount;
		
		static uint64_t allEntitiesAdded;
	};
	
	template<typename T>
	void EntityBufferManager::AddVector(std::vector<T>* vec) {
		buffers.push_back(BufferInfo{
			[](void* vec, uint32_t capacity) {
				((std::vector<T>*)vec)->reserve(capacity);
			},
			[](void* vec, uint32_t size) {
				((std::vector<T>*)vec)->resize(size);
			},
			nullptr,
			[](void* vec, uint32_t from, uint32_t to) {
				std::swap(((std::vector<T>*)vec)[0][from],
						((std::vector<T>*)vec)[0][to]);
			},
			nullptr,
			vec
		});
	}
}

#endif

