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

#ifndef QUICKGL_DELTA_VBO_MANAGER_HPP
#define QUICKGL_DELTA_VBO_MANAGER_HPP

#include <cinttypes>

#include <memory>
#include <vector>

namespace gl {
#ifndef OGLW_VBO_HPP
	class VBO;
#endif
}

namespace cql {
	class DeltaVboManager final {
	public:
		
		struct PairMove {
			uint32_t from;
			uint32_t to;
		};
		
		DeltaVboManager(uint32_t bytesPerVbo, uint32_t numberOfVbos);
		~DeltaVboManager();
		
		void Init();
		void Destroy();
		
		std::shared_ptr<gl::VBO> GetNextUpdateVBO();
		
	private:
		
		uint32_t lastUsedVbo;
		std::vector<std::shared_ptr<gl::VBO>> vbos;
	};
}

#endif


