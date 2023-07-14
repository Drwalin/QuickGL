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

#ifndef QUICKGL_GLOBAL_ENTITY_MANAGER_HPP
#define QUICKGL_GLOBAL_ENTITY_MANAGER_HPP

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "Engine.hpp"

namespace qgl {
	class Engine;
	class Pipeline;
	
	class GlobalEntityManager final {
	public:
		
		struct PipelineOffsetPair {
			uint32_t offset;
			uint32_t pipelineId;
		};
		
		GlobalEntityManager(std::shared_ptr<Engine> engine);
		~GlobalEntityManager();
		
		uint32_t GetNewEntity(std::shared_ptr<Pipeline> pipeline,
				uint32_t offset);
		void FreeEntity(uint32_t entity);
		
		uint32_t GetEntityOffset(uint32_t entity);
		template<typename T>
		std::shared_ptr<T> GetEntityPipeline(uint32_t entity);
		
		void SetEntityOffset(uint32_t entity, uint32_t offset);
		void SetEntityPipeline(uint32_t entity, uint32_t offset);
		
	private:
		
		std::shared_ptr<Engine> engine;
		
		uint32_t allEntitiesAdded;
		uint32_t lastAddedEntity;
		std::unordered_map<uint32_t, PipelineOffsetPair> mapEntity;
	};
	
	template<typename T>
	std::shared_ptr<T> GlobalEntityManager::GetEntityPipeline(uint32_t entity) {
		auto it = mapEntity.find(entity);
		if(it == mapEntity.end()) {
			return nullptr;
		}
		return std::dynamic_pointer_cast<T>(
				engine->GetPipeline(it->second.pipelineId));
	}
}

#endif

