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

#ifndef QUICKGL_POST_PROCESS_GENERATE_OCCLUSION_CULLING_DEPTH_MIPMAP_HPP
#define QUICKGL_POST_PROCESS_GENERATE_OCCLUSION_CULLING_DEPTH_MIPMAP_HPP

#include "PostProcess.hpp"

namespace qgl {
	class Camera;
	
	class PostProcessGenerateOcclusionCullingDepthMipmap : public PostProcess {
	public:
		
		PostProcessGenerateOcclusionCullingDepthMipmap();
		virtual ~PostProcessGenerateOcclusionCullingDepthMipmap();
		
		virtual void Execute(std::shared_ptr<Camera> camera) override;
		
	private:
		
		uint32_t textureLocation;
		uint32_t reduceSizeLocation;
		
		std::shared_ptr<gl::Shader> shader;
		std::shared_ptr<gl::VBO> vbo;
		std::shared_ptr<gl::VAO> vao;
		std::shared_ptr<gl::FBO> fbo;
	};
}

#endif

