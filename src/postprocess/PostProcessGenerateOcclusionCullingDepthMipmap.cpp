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
#extension GL_ARB_explicit_uniform_location : enable

layout ( location = 0 ) in vec2 pos;

out vec2 texCoord;
layout (location=4) uniform ivec2 srcImageSize;
uniform int reduceSize;

void main() {
	gl_Position = vec4(pos*reduceSize-1,1,1);
	texCoord = pos * vec2((srcImageSize+reduceSize-1)/reduceSize)*reduceSize;
}
)",


"",


R"(
#version 420 core
#extension GL_ARB_explicit_uniform_location : enable

in vec2 texCoord;

uniform sampler2D tex;
layout (location=4) uniform ivec2 srcImageSize;
layout (location=8) uniform int level;
uniform int reduceSize;

float SafeTexelFetch(ivec2 p) {
	if(p.x<0 || p.y<0 || p.x>=srcImageSize.x || p.y>=srcImageSize.y)
		return 1;
	return texelFetch(tex, p, level).x;
}

float GetValue(ivec2 p) {
	float a = SafeTexelFetch(p+ivec2(0,0));
	float b = SafeTexelFetch(p+ivec2(1,0));
	float c = SafeTexelFetch(p+ivec2(0,1));
	float d = SafeTexelFetch(p+ivec2(1,1));
	return max(max(a,b),max(c,d));
}

float GetFullValue(ivec2 p) {
	float ret = 0;
	for(int i=0; i<reduceSize; i+=2) {
		for(int j=0; j<reduceSize; j+=2) {
			ret = max(ret, GetValue(p+ivec2(i,j)));
		}
	}
	return ret;
}

void main() {
	vec2 p = texCoord;
	ivec2 pp = ivec2(p);
	gl_FragDepth = GetFullValue((pp|ivec2(1,1))^ivec2(1,1));
}
)");
		textureLocation = shader->GetUniformLocation("tex");
		reduceSizeLocation = shader->GetUniformLocation("reduceSize");
		
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
		int w=depthTexture->GetWidth(),
			h=depthTexture->GetHeight(),
			l=1;
		vao->Bind();
		glDepthFunc(GL_ALWAYS);
		int reduceSize = 2;
		for(; w>2 && h>2; ++l) {
			glFramebufferTexture2D(GL_FRAMEBUFFER, gl::ATTACHMENT_DEPTH,
					GL_TEXTURE_2D, depthTexture->GetTexture(), l);
			
			GL_CHECK_PUSH_PRINT_ERROR;

			glProgramUniform2i(shader->GetProgram(), 4, w, h);
			GL_CHECK_PUSH_PRINT_ERROR;
			glProgramUniform1i(shader->GetProgram(), 8, l-1);
			GL_CHECK_PUSH_PRINT_ERROR;
			glProgramUniform1i(shader->GetProgram(), reduceSizeLocation, reduceSize);
			
			w = (w + reduceSize-1)/reduceSize;
			h = (h + reduceSize-1)/reduceSize;
			glViewport(0, 0, w, h);
			GL_CHECK_PUSH_PRINT_ERROR;
			vao->DrawArrays(0, 4);
			GL_CHECK_PUSH_PRINT_ERROR;
			
			gl::MemoryBarrier(gl::ALL_BARRIER_BITS);
			
			reduceSize = std::min(2, reduceSize);
		}
		shader->Unuse();
		vao->Unbind();
		fbo->Unbind();
		glDepthFunc(GL_LESS);
	}
}

