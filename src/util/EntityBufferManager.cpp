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

#include <unordered_map>

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"

#include "../../include/quickgl/util/EntityBufferManager.hpp"

namespace qgl {
	EntityBufferManager::EntityBufferManager() {
		lastAddedEntity = 1;
	}
	
	EntityBufferManager::~EntityBufferManager() {
		Destroy();
	}
	
	void EntityBufferManager::Init() {
		deltaVbo = new gl::VBO(sizeof(PairMove), gl::SHADER_STORAGE_BUFFER,
				gl::DYNAMIC_DRAW);
		deltaVbo->Init();
		mapOffsetToEntity.Init();
	}
	
	void EntityBufferManager::Destroy() {
		delete deltaVbo;
		deltaVbo = nullptr;
		mapOffsetToEntity.Destroy();
		
		deltaFromTo.clear();
		deltaToFrom.clear();
		deltaBuffer.clear();
		freeingEntites.clear();
		mapEntityToOffset.clear();
		buffers.clear();
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
				buf.updateVbo(buf.data, stageId);
			}
		}
		if(stageId == 1) {
			GenerateDeltaBuffer();
			for(BufferInfo& buf : buffers) {
				if(buf.moveByVbo) {
					buf.moveByVbo(buf.data, deltaVbo);
				} else if(buf.moveByOne) {
					for(PairMove& p : deltaBuffer) {
						buf.moveByOne(buf.data, p.from, p.to);
					}
				}
			}
		}
	}
	
	
	void EntityBufferManager::AddVBO(gl::VBO* vbo) {
// 		buffers.push_back({
// 			[vbo](uint32_t capacity) { // reserve
// 				vbo->Resize(capacity);
// 			},
// 			[vbo](uint32_t size) { // resize
// 				vbo->Resize(size);
// 			},
// 			[vbo](gl::VBO* deltaVbo) { // update with delta buffer
// 				gl::DeltaBufferUpdater::GetByVertexSize(vbo->VertexSize())
// 					->Update(vbo, deltaVbo);
// 			},
// 			nullptr,
// 			nullptr,
// 			vbo
// 		});
		buffers.push_back(BufferInfo{
			[](void* vbo, uint32_t capacity) { // reserve
				((gl::VBO*)vbo)->Resize(capacity);
			},
			[](void* vbo, uint32_t size) { // resize
				((gl::VBO*)vbo)->Resize(size);
			},
			nullptr,
			[](void* vbo, uint32_t from, uint32_t to) {
				const uint32_t vs = ((gl::VBO*)vbo)->VertexSize();
				((gl::VBO*)vbo)->Copy(((gl::VBO*)vbo), from*vs, to*vs, vs);
			},
			nullptr,
			vbo
		});
	}
	
	void EntityBufferManager::AddManagedSparselyUpdateVBO(
			qgl::UntypedManagedSparselyUpdatedVBO* vbo) {
// 		buffers.push_back({
// 			[vbo](uint32_t capacity) { // reserve
// 				vbo->Resize(capacity);
// 			},
// 			[vbo](uint32_t size) { // resize
// 				vbo->Resize(size);
// 			},
// 			[vbo](gl::VBO* deltaVbo) { // update with delta buffer
// 				gl::DeltaBufferUpdater::GetByVertexSize(vbo->Vbo().VertexSize())
// 					->Update(vbo, deltaVbo);
// 			},
// 			nullptr,
// 			[vbo](uint32_t stageId) { // update vbo
// 				vbo->UpdateVBO(stageId);
// 			},
// 			vbo
// 		});
		buffers.push_back(BufferInfo{
			[](void* vbo, uint32_t capacity) { // reserve
				((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)->Resize(capacity);
			},
			[](void* vbo, uint32_t size) { // resize
				((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)->Resize(size);
			},
			nullptr,
			[](void* vbo, uint32_t from, uint32_t to) {
				const uint32_t vs
					= ((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)
						->Vbo().VertexSize();
				((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)
					->Vbo().Copy(
							&(((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)
								->Vbo()),
							from*vs, to*vs, vs);
			},
			[](void* vbo, uint32_t stageId) { // update vbo
				((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)
					->UpdateVBO(stageId);
			},
			vbo
			});
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
				deltaBuffer.push_back({it.first, it.second});
			}
		}
		
		deltaVbo->Update(deltaBuffer.data(), 0,
				deltaBuffer.size()*sizeof(PairMove));
		deltaFromTo.clear();
		deltaToFrom.clear();
		freeingEntites.clear();
	}
}

