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

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"

#include "../../include/quickgl/util/EntityBufferManager.hpp"
#include <unordered_map>

namespace qgl {
	EntityBufferManager::EntityBufferManager() {
		lastAddedEntity = 1;
	}
	
	EntityBufferManager::~EntityBufferManager() {
		Destroy();
	}
	
	void EntityBufferManager::Init() {
		throw "EntityBufferManager::Init() is unimplemented.";
	}
	
	uint32_t EntityBufferManager::GetNewEntity() {
		uint32_t entity = 0;
		for(;;) {
			entity = ++lastAddedEntity;
			if(entity == 0)
				continue;
			if(mapEntityToOffset.find(entity) == mapEntityToOffset.end())
				break;
		}
		
		uint32_t offset = entitiesBufferSize;
		entitiesBufferSize++;
		mapEntityToOffset[entity] = offset;
		mapOffsetToEntity.SetValue(offset, entity);
		
		return entity;
	}
	
	void EntityBufferManager::FreeEntity(uint32_t entity) {
		freeingEntites.emplace_back(entity);
	}
	
	void EntityBufferManager::UpdateBuffers(uint32_t stageId) {
		for(BufferInfo& buf : buffers) {
			if(buf.updateVbo) {
				buf.updateVbo(stageId);
			}
		}
		if(stageId == 1) {
			GenerateDeltaBuffer();
			for(BufferInfo& buf : buffers) {
				if(buf.moveByVbo) {
					buf.moveByVbo(deltaVbo);
				} else if(buf.moveByOne) {
					for(PairMove& p : deltaBuffer) {
						buf.moveByOne(p.from, p.to);
					}
				}
			}
		}
	}
	
	
	void EntityBufferManager::AddVBO(gl::VBO* vbo) {
		buffers.emplace_back(
			[vbo](uint32_t capacity) { // reserve
				vbo->Resize(capacity);
			},
			[vbo](uint32_t size) { // resize
				vbo->Resize(size);
			},
			[vbo](gl::VBO* deltaVbo) { // update with delta buffer
				gl::DeltaBufferUpdater::GetByVertexSize(vbo->VertexSize())
					->Update(vbo, deltaVbo);
			},
			nullptr,
			nullptr,
			vbo
		);
	}
	
	void EntityBufferManager::AddManagedSparselyUpdateVBO(
			qgl::UntypedManagedSparselyUpdatedVBO* vbo) {
		buffers.emplace_back(
			[vbo](uint32_t capacity) { // reserve
				vbo->Resize(capacity);
			},
			[vbo](uint32_t size) { // resize
				vbo->Resize(size);
			},
			[vbo](gl::VBO* deltaVbo) { // update with delta buffer
				gl::DeltaBufferUpdater::GetByVertexSize(vbo->Vbo().VertexSize())
					->Update(vbo, deltaVbo);
			},
			nullptr,
			[vbo](uint32_t stageId) { // resize
				vbo->UpdateVBO(stageId);
			},
			vbo
		);
	}
	
	uint32_t EntityBufferManager::GetOffsetOfEntity(uint32_t entity) const {
		return mapEntityToOffset.at(entity);
	}
	
	void EntityBufferManager::GenerateDeltaBuffer() {
		throw "EntityBufferManager::Init() is unimplemented.";
		
		for(uint32_t entity : freeingEntites) {
			uint32_t offset = mapEntityToOffset[entity];
			mapEntityToOffset.erase(entity);
			
			throw "EntityBufferManager::Init() is unimplemented.";
			
			// ...
			auto it = deltaFromTo.find(offset);
			if(it != deltaFromTo.end()) {
				deltaToFrom.erase(it->second);
				// ...
			} else {
				auto it = deltaFromTo.find(entitiesBufferSize);
				if(it != deltaFromTo.end()) {
					
				} else {
					
				}
			}
			// ...
		}
		
		
		
		
		
		
		
		deltaBuffer.clear();
		for(auto it : deltaFromTo) {
			if(it.first > 0) {
				deltaBuffer.emplace_back(it.first, it.second);
			}
		}
		
		deltaVbo->Update(deltaBuffer.data(), 0,
				deltaBuffer.size()*sizeof(PairMove));
		deltaFromTo.clear();
		deltaToFrom.clear();
		freeingEntites.clear();
	}
}

