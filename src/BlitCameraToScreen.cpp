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
#include "../OpenGLWrapper/include/openglwrapper/FBO.hpp"
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

void main() {
	FragColor = texture(tex, texCoord);
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
	}
	
	void BlitCameraToScreen::Blit(std::shared_ptr<gl::Texture> texture,
			uint32_t width, uint32_t height) {
		gl::VAO::Unbind();
		gl::FBO::Unbind();
		shader->Use();
		shader->SetTexture(textureLocation, texture.get(), 0);
		glViewport(0, 0, width, height);
		GL_CHECK_PUSH_PRINT_ERROR;
		glDisable(GL_DEPTH_TEST);
		vao->DrawArrays(0, 4);
		glEnable(GL_DEPTH_TEST);
		vao->Unbind();
	}
}

