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

#ifndef QUICKGL_IDS_MANAGER_HPP
#define QUICKGL_IDS_MANAGER_HPP

#include <cinttypes>

#include <vector>
#include <map>

namespace qgl {
	class IdsManager {
	public:
		
		uint32_t GetNewId();
		void FreeId(uint32_t id);
		
		inline uint32_t CountIds() const { return arrayOfUsedIds.size(); }
		inline uint32_t GetArraySize() const {
			return CountIds() + freeIdsStack.size();
		}
		
		void OptimizeIds();
		
		inline const uint32_t* GetArrayOfUsedIds() const {
			return &(arrayOfUsedIds[0]);
		}
		
	private:
		
		std::vector<uint32_t> ids;
		std::vector<uint32_t> freeIdsStack;
		
		std::vector<uint32_t> arrayOfUsedIds;
		std::map<uint32_t, uint32_t> mapIdToOffsetInArrayOfUsedIds;
	};
}

#endif

