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

#include "../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"
#include "../OpenGLWrapper/include/openglwrapper/Shader.hpp"
#include "../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../OpenGLWrapper/include/openglwrapper/VAO.hpp"
#include "../OpenGLWrapper/include/openglwrapper/Texture.hpp"

#include "../include/quickgl/cameras/Camera.hpp"

#include "../include/quickgl/BlitCameraToScreen.hpp"

namespace qgl {
	BlitCameraToScreen::BlitCameraToScreen() {
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

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D tex;
uniform int lod;
uniform vec4 coords;
uniform vec2 size;

void main() {
	vec2 uv = texCoord * coords.zw + coords.xy;
	ivec2 isize = ivec2(size) >> lod;
	ivec2 iuv = ivec2(uv * (isize));
	vec4 v = texelFetch(tex, iuv, lod);
	if(v.z > 0.01 || v.y > 0.01)
		FragColor = v;//*v*v*v*v*v*v*v;
	else {
		v.x = pow(v.x, 16);
		FragColor = v;
	}
}
)");
		textureLocation = shader->GetUniformLocation("tex");
		lodLocation = shader->GetUniformLocation("lod");
		coordsLocation = shader->GetUniformLocation("coords");
		sizeLocation = shader->GetUniformLocation("size");
		
		vbo = std::make_shared<gl::VBO>(2, gl::ARRAY_BUFFER, gl::STATIC_DRAW);
		uint8_t p[8] = {0, 0, 1, 0, 0, 1, 1, 1};
		vbo->Init();
		vbo->Generate(p, 4);
		
		vao = std::make_shared<gl::VAO>(gl::TRIANGLE_STRIP);
		vao->Init();
		vao->SetAttribPointer(*vbo, 0, 2, gl::BYTE, false, 0, 0);
	}
	
	void BlitCameraToScreen::Blit(std::shared_ptr<gl::Texture> texture,
			float srcX, float srcY,
			float srcW, float srcH,
			uint32_t dstX, uint32_t dstY,
			uint32_t dstW, uint32_t dstH,
			int lod) {
		vao->Bind();
		shader->Use();
		shader->SetTexture(textureLocation, texture.get(), 0);
		shader->SetInt(lodLocation, lod);
		shader->SetVec4(coordsLocation, {srcX, srcY, srcW, srcH});
		shader->SetVec2(sizeLocation, {texture->GetWidth(), texture->GetHeight()});
		glViewport(dstX, dstY, dstW, dstH);
		GL_CHECK_PUSH_PRINT_ERROR;
		glDisable(GL_DEPTH_TEST);
		vao->DrawArrays(0, 4);
		glEnable(GL_DEPTH_TEST);
		vao->Unbind();
	}
}

