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

#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"

#include "../../include/quickgl/util/DeltaVboManager.hpp"

namespace qgl {
	DeltaVboManager::DeltaVboManager(uint32_t bytesPerVbo,
			uint32_t numberOfVbos) {
		vboSize = bytesPerVbo;
		vbos.resize(numberOfVbos);
	}
	
	DeltaVboManager::~DeltaVboManager() {
		Destroy();
	}
	
	void DeltaVboManager::Init() {
		lastUsedVbo = 0;
		if(vbos[0] == nullptr) {
			for(std::shared_ptr<gl::VBO>& vbo : vbos) {
				vbo = std::make_shared<gl::VBO>(1, gl::SHADER_STORAGE_BUFFER,
						gl::DYNAMIC_DRAW);
				vbo->Init(vboSize);
			}
		}
	}
	
	void DeltaVboManager::Destroy() {
		for(std::shared_ptr<gl::VBO>& vbo : vbos) {
			vbo->Destroy();
			vbo = nullptr;
		}
	}
	
	std::shared_ptr<gl::VBO> DeltaVboManager::GetNextUpdateVBO() {
		std::shared_ptr<gl::VBO> ret = vbos[lastUsedVbo];
		lastUsedVbo = (lastUsedVbo+1) % vbos.size();
		return ret;
	}
}

