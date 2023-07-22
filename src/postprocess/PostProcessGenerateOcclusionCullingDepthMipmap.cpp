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

#include "../../OpenGLWrapper/include/openglwrapper/Shader.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Texture.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VAO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/FBO.hpp"

#include "../../include/quickgl/cameras/Camera.hpp"

#include "../../include/quickgl/postprocess/PostProcessGenerateOcclusionCullingDepthMipmap.hpp"

namespace qgl {
	PostProcessGenerateOcclusionCullingDepthMipmap
	::PostProcessGenerateOcclusionCullingDepthMipmap() {
		shader = std::make_shared<gl::Shader>();
		shader->Compile(R"(
#version 420 core

layout ( location = 0 ) in vec2 pos;

out vec2 texCoord;

void main() {
	gl_Position = vec4(pos*2-1,1,1);
	texCoord = pos;
}
)",


"",


R"(
#version 420 core
#extension GL_ARB_explicit_uniform_location : enable

in vec2 texCoord;

uniform sampler2D tex;
layout (location=4) uniform ivec2 sourceImageSize;
layout (location=8) uniform int level;

float GetValue(ivec2 p) {
	float a = texelFetch(tex, p*2+ivec2(0,0), level).x;
	float b = texelFetch(tex, p*2+ivec2(1,0), level).x;
	float c = texelFetch(tex, p*2+ivec2(0,1), level).x;
	float d = texelFetch(tex, p*2+ivec2(1,1), level).x;
	return max(max(a,b),max(c,d));
}

void main() {
	vec2 p = texCoord * vec2(sourceImageSize) + vec2(0.5, 0.5);
	ivec2 pp = ivec2(p);
	gl_FragDepth = GetValue(pp);
}
)");
		textureLocation = shader->GetUniformLocation("tex");
		
		vbo = std::make_shared<gl::VBO>(2, gl::ARRAY_BUFFER, gl::STATIC_DRAW);
		uint8_t p[8] = {0, 0, 1, 0, 0, 1, 1, 1};
		vbo->Init();
		vbo->Generate(p, 4);
		
		vao = std::make_shared<gl::VAO>(gl::TRIANGLE_STRIP);
		vao->Init();
		vao->SetAttribPointer(*vbo, 0, 2, gl::BYTE, false, 0, 0);
		
		fbo = std::make_shared<gl::FBO>();
	}

	PostProcessGenerateOcclusionCullingDepthMipmap
	::~PostProcessGenerateOcclusionCullingDepthMipmap() {
	}

	void PostProcessGenerateOcclusionCullingDepthMipmap
	::Execute(std::shared_ptr<Camera> camera) {
		auto depthTexture = camera->GetDepthTexture();
		if(depthTexture == nullptr)
			throw "PostProcessGenerateOcclusionCullingDepthMipmap camera->GetDepthTexture returned nullptr.";
		shader->Use();
		shader->SetTexture(textureLocation, depthTexture.get(), 0);
		
		fbo->SimpleBind();
		int w=(depthTexture->GetWidth()+1)/2,
			h=(depthTexture->GetHeight()+1)/2,
			l=1;
		vao->Bind();
		glDepthFunc(GL_ALWAYS);
		for(; w>2 && h>2; ++l, w=(w+1)/2, h=(h+1)/2) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, gl::ATTACHMENT_DEPTH,
					GL_TEXTURE_2D, depthTexture->GetTexture(), l);
			
			GL_CHECK_PUSH_PRINT_ERROR;

			glProgramUniform2i(shader->GetProgram(), 4, w, h);
			GL_CHECK_PUSH_PRINT_ERROR;
			glProgramUniform1i(shader->GetProgram(), 8, l-1);
			GL_CHECK_PUSH_PRINT_ERROR;
			
			glViewport(0, 0, w, h);
			GL_CHECK_PUSH_PRINT_ERROR;
			vao->DrawArrays(0, 4);
			GL_CHECK_PUSH_PRINT_ERROR;
			
			gl::MemoryBarrier(gl::ALL_BARRIER_BITS);
		}
		shader->Unuse();
		vao->Unbind();
		fbo->Unbind();
		glDepthFunc(GL_LESS);
	}
}

