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

#include <cmath>

#include "../../include/quickgl/util/IdsManager.hpp"

namespace qgl {
	
	uint32_t IdsManager::GetNewId() {
		if(freeIdsStack.size()) {
			uint32_t id = freeIdsStack.back();
			freeIdsStack.resize(freeIdsStack.size()-1);
			uint32_t arrayOfUsedIdsOffset = arrayOfUsedIds.size();
			mapIdToOffsetInArrayOfUsedIds[id] = arrayOfUsedIdsOffset;
			arrayOfUsedIds.emplace_back(id);
			return id;
		} else {
			uint32_t id = ids.size();
			ids.resize(id+1);
			arrayOfUsedIds.push_back(id);
			mapIdToOffsetInArrayOfUsedIds[id] = id; 
			return id;
		}
	}
	
	void IdsManager::FreeId(uint32_t id) {
		freeIdsStack.emplace_back(id);
		uint32_t arrayOfUsedIdsOffset = mapIdToOffsetInArrayOfUsedIds[id];
		mapIdToOffsetInArrayOfUsedIds.erase(id);
		
		if(arrayOfUsedIdsOffset != arrayOfUsedIds.size()-1) {
			uint32_t movingId = arrayOfUsedIds.back();
			arrayOfUsedIds[arrayOfUsedIdsOffset] = movingId;
			mapIdToOffsetInArrayOfUsedIds[movingId] = arrayOfUsedIdsOffset;
		}
		
		arrayOfUsedIds.resize(arrayOfUsedIds.size()-1);
	}
	
	
	
	
	
	void IdsManagerVBOManaged::InitVBO() {
		vbo.Init();
	}
	
	uint32_t IdsManagerVBOManaged::GetNewId() {
		if(freeIdsStack.empty()) {
			const uint32_t addSize = std::max<uint32_t>(256, (arrayOfUsedIds.size()*3)/2);
			const uint32_t start = arrayOfUsedIds.size();
			arrayOfUsedIds.reserve(arrayOfUsedIds.size()+addSize);
			mapIdToOffsetInArrayOfUsedIds.resize(arrayOfUsedIds.size()+addSize);
			freeIdsStack.resize(addSize);
			for(int i=0; i<addSize; ++i) {
				freeIdsStack[i] = start+addSize-1-i;
			}
		}
		uint32_t id = freeIdsStack.back();
		freeIdsStack.resize(freeIdsStack.size()-1);
		uint32_t arrayOfUsedIdsOffset = arrayOfUsedIds.size();
		mapIdToOffsetInArrayOfUsedIds[id] = arrayOfUsedIdsOffset;
		vbo.SetValue(id, arrayOfUsedIds.size());
		arrayOfUsedIds.emplace_back(id);
		return id;
	}
	
	void IdsManagerVBOManaged::FreeId(uint32_t id) {
		freeIdsStack.emplace_back(id);
		uint32_t arrayOfUsedIdsOffset = mapIdToOffsetInArrayOfUsedIds[id];
		
		if(arrayOfUsedIdsOffset != arrayOfUsedIds.size()-1) {
			uint32_t movingId = arrayOfUsedIds.back();
			vbo.SetValue(movingId, arrayOfUsedIdsOffset);
			arrayOfUsedIds[arrayOfUsedIdsOffset] = movingId;
			mapIdToOffsetInArrayOfUsedIds[movingId] = arrayOfUsedIdsOffset;
		}
		
		arrayOfUsedIds.resize(arrayOfUsedIds.size()-1);
	}
	
	uint32_t IdsManagerVBOManaged::UpdateVBO(uint32_t stageId) {
		return vbo.UpdateVBO(stageId);
	}
}

