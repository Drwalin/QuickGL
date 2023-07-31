
#pragma once

#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/VBO.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Texture.hpp"
#include "../../OpenGLWrapper/include/openglwrapper/Sync.hpp"

#include "../../include/quickgl/Engine.hpp"
#include "../../include/quickgl/MeshManager.hpp"
#include "../../include/quickgl/pipelines/PipelineStatic.hpp"
#include "../../include/quickgl/pipelines/PipelineBoneAnimated.hpp"
#include "../../include/quickgl/cameras/FreeFlyCamera.hpp"
#include "../../include/quickgl/Gui.hpp"

#include "../../include/quickgl/util/Log.hpp"
#include "quickgl/util/EntityBufferManager.hpp"

#include <ctime>
#include <cstdio>

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <GL/glext.h>

#define PRINT_PARAMETER(X) {int v=0; glGetIntegerv(X, &v); printf(" %s = %i\n", #X, v); fflush(stdout);}

#include <thread>
#include <chrono>
#include <random>
#include <algorithm>

#include "DrwalinLibraries/Perlin.h"

class TerrainGenerator {
public:
	
	std::shared_ptr<qgl::Engine> engine;
	std::shared_ptr<qgl::PipelineStatic> pipelineStatic;
	std::shared_ptr<qgl::PipelineBoneAnimated> pipelineAnimated;
	
	Perlin perlin;
	Perlin perlinForest;
	std::random_device rd;
	
	inline const static int chunkSize = 32;
	inline const static float _dxy = 0.002;
	inline const static float dh = 100;
	
	float GetHeight(float x, float y) {
		x *= _dxy;
		y *= _dxy;
		return perlin.Value(x, y) * dh;
	}
	
	glm::vec3 GetNormal(float x, float y) {
		float dx = GetHeight(x, y+0.5) - GetHeight(x, y-0.5);
		float dy = GetHeight(x+0.5, y) - GetHeight(x-0.5, y);
		glm::vec3 ret;
		
		glm::vec3 a{0, dx, 1}, b{1, dy, 0};
		
		ret = glm::normalize(glm::cross(a, b));
		if(ret.y < 0)
			ret = -ret;
		return ret;
		
		return glm::normalize(
			glm::rotate(
				glm::rotate(
					glm::vec3(-dx,1,-dy), -3.141592f/2, {1,0,0}),
				-3.141592f/2, {0,0,1}));
		
		
		ret.x = 0;
		ret.y = 1;
		ret.z = dy;
		return glm::normalize(-ret);
	}
	
	glm::quat GetRotationQuatFromNormal(float x, float y) {
		glm::vec3 up{0,1,0}, normal = GetNormal(x, y);
		return glm::orientation(normal, up);
	}
	
	float GetDensity(float x, float y) {
		float dxy = _dxy*10;
		return perlinForest.Value(x*dxy, y*dxy);
	}
	
	class ChunkGenerator {
	public:
		
		std::random_device rd;
		
		float RandomNormal(float min, float max) {
			std::uniform_real_distribution<float> d(0, 0.25);
			return (/*d(rd) + d(rd) + d(rd)*/ + d(rd)) * (max-min) + min;
		}
		
		glm::vec4 RandomColor(glm::vec4 base, float delta) {
			base.r = std::clamp(base.r + RandomNormal(-1, 1) * delta, 0.0f, 1.0f);
			base.g = std::clamp(base.g + RandomNormal(-1, 1) * delta, 0.0f, 1.0f);
			base.b = std::clamp(base.b + RandomNormal(-1, 1) * delta, 0.0f, 1.0f);
			base.a = 1;
			return base;
		}
		
		std::vector<glm::vec3> pos;
		std::vector<glm::vec3> normal;
		std::vector<std::vector<glm::vec4>> color;
		std::vector<std::vector<glm::vec2>> uv;
		std::vector<uint32_t> indices;
		std::string name;
		
		TerrainGenerator* gn;
		
		void AddQuad(int xs, int ys, float x, float y) {
			xs += chunkSize/2;
			ys += chunkSize/2;
			uint32_t startId = pos.size();
			
			float h1 = gn->GetHeight(x, y);
			glm::vec3 a(x-xs, h1, y-ys);
			float h2 = gn->GetHeight(x+1, y);
			glm::vec3 b(x+1-xs, h2, y-ys);
			float h3 = gn->GetHeight(x, y+1);
			glm::vec3 c(x-xs, h3, y+1-ys);
			float h4 = gn->GetHeight(x+1, y+1);
			glm::vec3 d(x+1-xs, h4, y+1-ys);
			
			glm::vec3 n1 = glm::normalize(glm::cross(a-b, c-b));
			if(n1.y < 0)
				n1 = -n1;
			
			glm::vec3 n2 = glm::normalize(glm::cross(b-c, d-c));
			if(n2.y < 0)
				n2 = -n2;
			
			pos.push_back(a);
			pos.push_back(b);
			pos.push_back(c);
			pos.push_back(d);
			
			normal.push_back(n1);
			normal.push_back(n1);
			normal.push_back(n2);
			normal.push_back(n2);
			
			indices.push_back(startId);
			indices.push_back(startId+1);
			indices.push_back(startId+2);
			indices.push_back(startId+1);
			indices.push_back(startId+2);
			indices.push_back(startId+3);
			
			glm::vec4 cc = RandomColor({0.1, (h1/dh+1)/2.5+0.1, 0.15, 1}, 0.1);
			color[0].push_back(cc);
			color[0].push_back(cc);
			color[0].push_back(cc);
			color[0].push_back(cc);
			
			uv[0].emplace_back();
			uv[0].emplace_back();
			uv[0].emplace_back();
			uv[0].emplace_back();
		}
		
		void Restart(int x, int y) {
			pos.clear();
			normal.clear();
			indices.clear();
			color.resize(1);
			uv.resize(1);
			color[0].clear();
			uv[0].clear();
			name = std::string("chunk") + std::to_string(x) + "_"
				+ std::to_string(y);
		}
		
	} cg;
	
public:
	
	TerrainGenerator(std::shared_ptr<qgl::Engine> engine,
			std::shared_ptr<qgl::PipelineStatic> pipelineStatic,
			std::shared_ptr<qgl::PipelineBoneAnimated> pipelineAnimated
			) :
		engine(engine), pipelineStatic(pipelineStatic),
		pipelineAnimated(pipelineAnimated) {
		perlin.Seed(rd(), 7);
		perlinForest.Seed(rd(), 3);
		cg.gn = this;
	}
	
	void Generate(int min, int max) {
		for(int chunkX=min; chunkX<max; chunkX+=chunkSize) {
			for(int chunkY=min; chunkY<max; chunkY+=chunkSize) {
				GenerateChunk(chunkX, chunkY);
				GenerateTrees(chunkX, chunkY);
			}
		}
	}
	
	void GenerateChunk(int xs, int ys) {
		cg.Restart(xs, ys);
		for(int x=xs; x<xs+chunkSize; ++x) {
			for(int y=ys; y<ys+chunkSize; ++y) {
				cg.AddQuad(xs, ys, x, y);
			}
		}
		
		auto mesh = pipelineStatic->GetMeshManager()->CreateMeshFromData(
				cg.name,
				cg.pos,
				cg.normal,
				cg.color,
				cg.uv,
				cg.indices,
				1);
		auto entity = pipelineStatic->CreateEntity();
		pipelineStatic->SetEntityMesh(entity, mesh);
		pipelineStatic->SetEntityTransformsQuat(entity,
				{xs+chunkSize/2, -50, ys+chunkSize/2});
	}
	
	void GenerateTrees(int xs, int ys) {
		std::uniform_real_distribution<float> d(-0.7, 0.7);
		std::uniform_real_distribution<float> e(0, 3.14159*2);
		std::uniform_real_distribution<float> s(0.0, 1);
		std::uniform_real_distribution<float> f(0, 5);
		
		for(int x=xs; x<xs+chunkSize; ++x) {
			for(int y=ys; y<ys+chunkSize; ++y) {
				const float v = this->GetDensity(x, y);
				if(v > f(rd)) {
					glm::vec3 p = {x+d(rd), 0, y+d(rd)};
					p.y = -50+GetHeight(p.x, p.z);
					float scale = (s(rd)*s(rd)*s(rd)*s(rd)+0.05) * 5 + 0.1;
					if(rand()%2) {
						uint32_t standId = pipelineStatic->CreateEntity();
						pipelineStatic->SetEntityMeshByName(standId, "fireStand");
						pipelineStatic->SetEntityTransformsQuat(standId,
								p,
								GetRotationQuatFromNormal(p.x, p.z)
									* glm::angleAxis(e(rd), glm::vec3{0,1,0}),
								glm::vec3(1,1,1)*scale
								);
					} else {
						uint32_t entity = pipelineAnimated->CreateEntity();
						pipelineAnimated->SetEntityMesh(entity, 1+rand()%2);
						pipelineAnimated->SetAnimationState(entity, rand()%4, rand()/300.0f, true, rand()%4, true);
						pipelineAnimated->SetEntityTransformsQuat(entity,
								p,
								GetRotationQuatFromNormal(p.x, p.z)
									* glm::angleAxis(e(rd), glm::vec3{0,1,0}),
								glm::vec3(1,1,1)*scale
								);
					}
					
				}
			}
		}
	}
};

