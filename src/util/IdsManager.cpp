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
}

