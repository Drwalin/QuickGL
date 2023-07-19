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

#include "../include/quickgl/pipelines/Pipeline.hpp"

#include "../include/quickgl/GlobalEntityManager.hpp"

namespace qgl {
	GlobalEntityManager::GlobalEntityManager(std::shared_ptr<Engine> engine) :
			engine(engine) {
		lastAddedEntity = 0;
	}

	GlobalEntityManager::~GlobalEntityManager() {
	}

	uint32_t GlobalEntityManager::GetNewEntity(
			std::shared_ptr<Pipeline> pipeline, uint32_t offset) {
		allEntitiesAdded++;
		uint32_t entity = 0;
		for(;;) {
			entity = ++lastAddedEntity;
			if(entity == 0)
				continue;
			if(mapEntity.find(entity) == mapEntity.end())
				break;
		}
		mapEntity[entity] = {offset, pipeline->GetPipelineId()};
		return entity;
	}

	void GlobalEntityManager::FreeEntity(uint32_t entity) {
		mapEntity.erase(entity);
	}

	uint32_t GlobalEntityManager::GetEntityOffset(uint32_t entity) {
		auto it = mapEntity.find(entity);
		if(it == mapEntity.end())
			return 0;
		return it->second.offset;
	}

	void GlobalEntityManager::SetEntityOffset(uint32_t entity,
			uint32_t offset) {
		mapEntity[entity].offset = offset;
	}
}

