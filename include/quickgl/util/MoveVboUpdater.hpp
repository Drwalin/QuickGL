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

#ifndef QUICKGL_MOVE_VBO_UPDATER_HPP
#define QUICKGL_MOVE_VBO_UPDATER_HPP

#include <cinttypes>

#include <memory>
#include <unordered_map>

namespace gl {
#ifndef OGLW_VBO_HPP
	class VBO;
#endif
#ifndef OGLW_Shader_HPP
	class Shader;
#endif
}

namespace qgl {
	class Engine;
	
	class MoveVboUpdater final {
	public:
		
		struct PairMove {
			uint32_t from;
			uint32_t to;
		};
		
		MoveVboUpdater(std::shared_ptr<Engine> engine, uint32_t bytes);
		~MoveVboUpdater();
		
		void Init();
		void Destroy();
		
		void Update(gl::VBO* vbo, const PairMove* data, uint32_t elements);
		
		void Update(gl::VBO* vbo, gl::VBO* deltaVbo, uint32_t elements);
		
		friend class MoveVboManager;
		
	private:
		
		const uint32_t BYTES;
		std::shared_ptr<Engine> engine;
		std::shared_ptr<gl::Shader> shader;
		uint32_t updateElementsCountLocation;
	};
	
	class MoveVboManager final {
	public:
		
		MoveVboManager(std::shared_ptr<Engine> engine);
		~MoveVboManager();
		
		void Destroy();
		
		std::shared_ptr<MoveVboUpdater> GetByObjectSize(uint32_t bytes);
		
		void Update(gl::VBO* vbo, const MoveVboUpdater::PairMove* data,
				uint32_t elements, uint32_t elementSize);
		
		void Update(gl::VBO* vbo, gl::VBO* deltaVbo, uint32_t elements,
				uint32_t elementSize);
		
	private:
		
		std::unordered_map<uint32_t, std::shared_ptr<MoveVboUpdater>> updatersByElementSizeSize;
		std::shared_ptr<Engine> engine;
	};
}

#endif

