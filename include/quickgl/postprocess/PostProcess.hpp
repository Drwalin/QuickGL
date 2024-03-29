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

#ifndef QUICKGL_POST_PROCESS_HPP
#define QUICKGL_POST_PROCESS_HPP

#include <memory>

namespace gl {
	class Shader;
	class VBO;
	class VAO;
	class FBO;
	class Texture;
}

namespace qgl {
	class Camera;
	
	class PostProcess : public std::enable_shared_from_this<PostProcess> {
	public:
		
		PostProcess();
		virtual ~PostProcess();
		
		virtual void Execute(std::shared_ptr<Camera> camera) = 0;
	};
}

#endif

