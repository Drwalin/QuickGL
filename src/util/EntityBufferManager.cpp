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

#include <cstdio>

#include <unordered_map>

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

#include "../../include/quickgl/util/EntityBufferManager.hpp"

namespace qgl {
	EntityBufferManager::EntityBufferManager() {
	}
	
	EntityBufferManager::~EntityBufferManager() {
		Destroy();
	}
	
	void EntityBufferManager::Init() {
// 		deltaVbo = new gl::VBO(sizeof(PairMove), gl::SHADER_STORAGE_BUFFER,
// 				gl::DYNAMIC_DRAW);
// 		deltaVbo->Init();
		mapOffsetToEntity.Init();
		lastAddedEntity = 1;
		entitiesCount = 0;
		entitiesBufferSize = 0;
	}
	
	void EntityBufferManager::Destroy() {
// 		delete deltaVbo;
// 		deltaVbo = nullptr;
		mapOffsetToEntity.Destroy();
		
// 		deltaFromTo.clear();
// 		deltaToFrom.clear();
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
		mapEntityToOffset[entity] = offset;
		mapOffsetToEntity.SetValue(entity, offset);
		
		entitiesBufferSize++;
		entitiesCount++;
		
		return entity;
	}
	
	void EntityBufferManager::FreeEntity(uint32_t entity) {
		freeingEntites.emplace_back(entity);
		entitiesCount--;
	}
	
	uint32_t EntityBufferManager::Count() const {
		return entitiesCount;
	}
	
	uint32_t EntityBufferManager::UpdateBuffers(uint32_t stageId) {
		for(BufferInfo& buf : buffers) {
			if(buf.updateVbo) {
				buf.updateVbo(buf.data, stageId);
			}
		}
		if(stageId == 1) {
			GenerateDeltaBuffer();
			for(BufferInfo& buf : buffers) {
				if(buf.moveByVbo) {
					throw "EntityBufferManager::UpdateBuffers() update by VBO is not implemented.";
// 					buf.moveByVbo(buf.data, deltaVbo);
				} else if(buf.moveByOne) {
					for(PairMove& p : deltaBuffer) {
						buf.moveByOne(buf.data, p.from, p.to);
					}
				}
			}
		}
		if(stageId == 0)
			return 1;
		return 0;
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
			[](void* vbo, uint32_t from, uint32_t to) { // move by one
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
			[](void* vbo, uint32_t from, uint32_t to) { // move by one
				const uint32_t vs
					= ((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)
						->Vbo().VertexSize();
				((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)
					->Vbo().Copy(
							&(((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)
								->Vbo()),
							from*vs, to*vs, vs);
				if(from == to) {
					printf("%u -> %u    [%u]  / %u\n", from, to, vs, 
							((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)->Count());
					fflush(stdout);
					gl::openGL.PrintErrors();
					gl::openGL.ClearErrors();
				}
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
		deltaBuffer.clear();
		std::unordered_map<uint32_t, PairMove> moveTo;
		moveTo.clear();
		for(const uint32_t entity : freeingEntites) {
			const uint32_t offset = mapEntityToOffset[entity];
			mapEntityToOffset.erase(entity);
			moveTo.erase(entity);
			
			entitiesBufferSize--;
			
			if(entitiesBufferSize == offset) {
				continue;
			}
			
			const uint32_t otherOffset = entitiesBufferSize;
			const uint32_t otherEntity = mapOffsetToEntity.GetValue(otherOffset);
			
			deltaBuffer.push_back({otherOffset, offset});
			
			mapOffsetToEntity.SetValue(otherEntity, offset);
			mapEntityToOffset[otherEntity] = offset;
			
			auto it = moveTo.find(otherEntity);
			if(it == moveTo.end()) {
				moveTo[otherEntity] = {otherOffset, offset};
			} else {
				it->second.to = offset;
			}
		}
		
		
		std::unordered_set<uint32_t> s;
		s.reserve(moveTo.size()*2);
		for(auto it : moveTo) {
			s.insert(it.second.from);
			s.insert(it.second.to);
		}
		if(s.size() != moveTo.size()*2) {
			printf(" s.size() = %lu == %lu\n", s.size(), moveTo.size()*2);
		}
		
		
		
		deltaBuffer.clear();
		for(auto it : moveTo) {
			if(it.first > 0) {
				deltaBuffer.push_back(it.second);
			}
		}
		
		
// 		deltaVbo->Update(deltaBuffer.data(), 0,
// 				deltaBuffer.size()*sizeof(PairMove));
// 		deltaFromTo.clear();
		freeingEntites.clear();
	}
}

