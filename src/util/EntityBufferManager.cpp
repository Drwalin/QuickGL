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

#include "../../include/quickgl/Engine.hpp"
#include "../include/quickgl/util/DeltaVboManager.hpp"
#include "../include/quickgl/util/MoveVboUpdater.hpp"

#include "../../include/quickgl/util/EntityBufferManager.hpp"

namespace qgl {
	EntityBufferManager::EntityBufferManager(std::shared_ptr<Engine> engine) :
		mapOffsetToEntity(engine), engine(engine) {
	}
	
	uint64_t EntityBufferManager::allEntitiesAdded = 0;
	
	EntityBufferManager::~EntityBufferManager() {
		Destroy();
	}
	
	void EntityBufferManager::Init() {
		mapOffsetToEntity.Init();
		lastAddedEntity = 1;
		entitiesCount = 0;
		entitiesBufferSize = 0;
	}
	
	void EntityBufferManager::Destroy() {
		mapOffsetToEntity.Destroy();
		
		deltaFromTo.clear();
		deltaBuffer.clear();
		freeingEntites.clear();
		mapEntityToOffset.clear();
		buffers.clear();
	}
	
	uint32_t EntityBufferManager::GetNewEntity() {
		allEntitiesAdded++;
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
	
	void EntityBufferManager::UpdateBuffers() {
		if(freeingEntites.empty()) {
			return;
		}
		GenerateDeltaBuffer();
		
		const uint32_t elements = deltaBuffer.size();
		if(elements < 128) {
			for(BufferInfo& buf : buffers) {
				for(PairMove& p : deltaBuffer) {
					buf.moveByOne(buf.data, p.from, p.to);
				}
			}
		} else {
			for(uint32_t i=0; i<elements;) {
				auto deltaVbo = engine->GetDeltaVboManager()
					->GetNextUpdateVBO();
				const uint32_t elem = std::min<uint32_t>(
						deltaVbo->GetVertexCount()/sizeof(PairMove),
						elements-i);
				
				deltaVbo->Update(&(deltaBuffer[i]), 0, elem*sizeof(PairMove));
				gl::MemoryBarrier(gl::BUFFER_UPDATE_BARRIER_BIT);
				
				for(BufferInfo& buf : buffers) {
					if(buf.moveByVbo) {
						buf.moveByVbo(buf.data, engine, deltaVbo.get(), elem);
					}
				}
				gl::Flush();
				
				for(BufferInfo& buf : buffers) {
					if(!buf.moveByVbo) {
						uint32_t end = i+elem;
						for(uint32_t j=i; j<end; ++j) {
							PairMove p = deltaBuffer[j];
							buf.moveByOne(buf.data, p.from, p.to);
						}
					}
				}
				
				i += elem;
			}
		}
	}
	
	
	void EntityBufferManager::AddVBO(gl::VBO* vbo) {
		buffers.push_back(BufferInfo{
			[](void* vbo, uint32_t capacity) { // reserve
				((gl::VBO*)vbo)->Resize(capacity);
			},
			[](void* vbo, uint32_t size) { // resize
				((gl::VBO*)vbo)->Resize(size);
			},
			[](void* vbo, std::shared_ptr<Engine> engine, gl::VBO* deltaVbo, uint32_t elements) { // update with delta buffer
				engine->GetMoveVboManager()->Update(((gl::VBO*)vbo),
						deltaVbo, elements, ((gl::VBO*)vbo)->VertexSize());
			},
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
		buffers.push_back(BufferInfo{
			[](void* vbo, uint32_t capacity) { // reserve
				((UntypedManagedSparselyUpdatedVBO*)vbo)->Resize(capacity);
			},
			[](void* vbo, uint32_t size) { // resize
				((UntypedManagedSparselyUpdatedVBO*)vbo)->Resize(size);
			},
			[](void* vbo, std::shared_ptr<Engine> engine, gl::VBO* deltaVbo, uint32_t elements) { // update with delta buffer
				engine->GetMoveVboManager()->Update(
						&(((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)->Vbo()),
						deltaVbo, elements,
						((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)->Vbo()
							.VertexSize());
			},
			[](void* vbo, uint32_t from, uint32_t to) { // move by one
				const uint32_t vs
					= ((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)
						->Vbo().VertexSize();
				((UntypedManagedSparselyUpdatedVBO*)vbo)
					->Vbo().Copy(
							&(((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)
								->Vbo()),
							from*vs, to*vs, vs);
			},
			[](void* vbo) { // update vbo
				((qgl::UntypedManagedSparselyUpdatedVBO*)vbo)
					->UpdateVBO();
			},
			vbo
			});
	}
	
	uint32_t EntityBufferManager::GetOffsetOfEntity(uint32_t entity) const {
		return mapEntityToOffset.at(entity);
	}
	
	void EntityBufferManager::GenerateDeltaBuffer() {
		deltaBuffer.clear();
		deltaFromTo.clear();
		for(const uint32_t entity : freeingEntites) {
			const uint32_t offset = mapEntityToOffset[entity];
			mapEntityToOffset.erase(entity);
			deltaFromTo.erase(entity);
			entitiesBufferSize--;
			
			if(entitiesBufferSize == offset) {
				continue;
			}
			
			const uint32_t otherOffset = entitiesBufferSize;
			const uint32_t otherEntity = mapOffsetToEntity.GetValue(otherOffset);
			
			mapOffsetToEntity.SetValue(otherEntity, offset);
			mapEntityToOffset[otherEntity] = offset;
			
			auto it = deltaFromTo.find(otherEntity);
			if(it == deltaFromTo.end()) {
				deltaFromTo[otherEntity] = {otherOffset, offset};
			} else {
				it->second.to = offset;
			}
		}
		
		for(auto it : deltaFromTo) {
			if(it.first > 0) {
				deltaBuffer.push_back(it.second);
			}
		}
		
		deltaFromTo.clear();
		freeingEntites.clear();
	}
}

