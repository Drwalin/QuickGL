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

#include <chrono>
#include <set>

#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

#include "../../include/quickgl/cameras/Camera.hpp"

#include "../../include/quickgl/util/RenderStageComposer.hpp"

namespace qgl {
	
	void RenderStageComposer::AddPipelineStages(
			std::shared_ptr<Pipeline> pipeline) {
		std::vector<std::shared_ptr<Stage>> renderStagesPtr;
		{
			std::vector<Stage> renderStages;
			pipeline->GenerateRenderStages(renderStages);
			renderStagesPtr.resize(renderStages.size());
			for(int i=0; i<renderStages.size(); ++i) {
				Stage& s = renderStages[i];
				s.pipeline = pipeline;
				if(s.canBeContinued == nullptr) {
					s.canBeContinued = [](std::shared_ptr<Camera>) { return true; };
				}
				renderStagesPtr[i] = std::make_shared<Stage>(std::move(s));
			}
		}
		for(int i=0; i<renderStagesPtr.size(); ++i) {
			if(i+1 < renderStagesPtr.size()) {
				renderStagesPtr[i]->nextStage = renderStagesPtr[i+1];
			}
		}
		for(int i=0; i<renderStagesPtr.size(); ++i) {
			if(renderStagesPtr[i]->stageType != STAGE_GLOBAL) {
				firstStageForCameraPerPipeline[renderStagesPtr[i]->pipeline]
					= renderStagesPtr[i];
			}
		}
		stagingPipelineTemplate.emplace_back(renderStagesPtr[0]);
	}
	
	void RenderStageComposer::RestartStages(
			std::vector<std::shared_ptr<Camera>> cameras) {
		std::shared_ptr<Camera> cam = nullptr;
		for(auto camera : cameras) {
			currentStagesPerCamera.clear();
			currentStagesPerFbo.clear();
			currentStagesGlobal = stagingPipelineTemplate;
			camerasRenderingOrder[cam] = camera;
			cam = camera;
		}
		camerasRenderingOrder[cam] = nullptr;
		timings.clear();
	}
	
	void RenderStageComposer::ContinueStages() {
		ContinueStagesGlobal();
		gl::Flush();
		ContinueStagesPerCamera();
		gl::Flush();
		ContinueStagesPerFbo();
		gl::Flush();
	}       
	        
	void RenderStageComposer::ContinueStagesGlobal() {
		std::vector<std::shared_ptr<Stage>> stages;
		stages.reserve(currentStagesGlobal.size());
		for(auto s : currentStagesGlobal) {
			if(s->canBeContinued(nullptr)) {
				auto begin = std::chrono::steady_clock::now();
				s->renderFunction(nullptr);
				auto next = s->nextStage;
				if(next) {
					switch(next->stageType) {
					case STAGE_GLOBAL:
						stages.emplace_back(next);
						break;
					case STAGE_PER_CAMERA:
						currentStagesPerCamera
							.emplace_back(next, camerasRenderingOrder[nullptr]);
						break;
					case STAGE_PER_CAMERA_FBO:
						currentStagesPerCamera
							.emplace_back(next, camerasRenderingOrder[nullptr]);
						break;
					}
				}
				auto end = std::chrono::steady_clock::now();
				uint64_t t = (end-begin).count();
				timings.push_back({s->pipeline, s, nullptr, t});
			} else {
				stages.emplace_back(s);
			}
		}
		std::swap(stages, currentStagesGlobal);
	}
	
	void RenderStageComposer::ContinueStagesPerCamera() {
		std::vector<StageCameraPair> stages;
		stages.reserve(currentStagesPerCamera.size());
		for(auto p : currentStagesPerCamera) {
			if(p.stage->canBeContinued(p.camera)) {
				auto begin = std::chrono::steady_clock::now();
				p.stage->renderFunction(p.camera);
				auto next = p.stage->nextStage;
				if(next) {
					switch(next->stageType) {
					case STAGE_GLOBAL:
						throw std::string("qgl::RenderStageComposer: Cannot run STAGE_GLOBAL after STAGE_PER_CAMERA");
						break;
					case STAGE_PER_CAMERA:
						stages.emplace_back(next, p.camera);
						break;
					case STAGE_PER_CAMERA_FBO:
						currentStagesPerCamera.emplace_back(next, p.camera);
						break;
					}
				} else {
					stages.emplace_back(
							firstStageForCameraPerPipeline[p.stage->pipeline],
							camerasRenderingOrder[p.camera]);
					
					auto c = camerasRenderingOrder[p.camera];
					if(c) {
						auto s = firstStageForCameraPerPipeline[p.stage->pipeline];
						switch(s->stageType) {
							case STAGE_GLOBAL:
								throw std::string("qgl::RenderStageComposer: Cannot run STAGE_GLOBAL for new camera");
								break;
							case STAGE_PER_CAMERA:
								currentStagesPerCamera.emplace_back(s, c);
								break;
							case STAGE_PER_CAMERA_FBO:
								throw std::string("qgl::RenderStageComposer: Cannot run STAGE_PER_CAMERA_FBO for new camera when last stage was STAGE_PER_CAMERA");
								break;
						}
					}
				}
				auto end = std::chrono::steady_clock::now();
				uint64_t t = (end-begin).count();
				timings.push_back({p.stage->pipeline, p.stage, p.camera, t});
			} else {
				stages.emplace_back(p);
			}
		}
		std::swap(stages, currentStagesPerCamera);
	}
	
	void RenderStageComposer::ContinueStagesPerFbo() {
		std::unordered_map<std::shared_ptr<Camera>,
			std::vector<std::shared_ptr<Stage>>> camerasInUse;
		{
			std::vector<StageCameraPair> stages = currentStagesPerFbo;
			for(auto& p : stages) {
				if(p.stage->canBeContinued(p.camera)) {
					camerasInUse[p.camera].emplace_back(p.stage);
				} else {
					currentStagesPerFbo.emplace_back(p);
				}
			}
		}
		currentStagesPerFbo.clear();
		for(auto& cam : camerasInUse) {
			auto camera = cam.first;
			camera->UseFbo();
			for(auto& stage : cam.second) {
				auto begin = std::chrono::steady_clock::now();
				stage->renderFunction(camera);
				auto next = stage->nextStage;
				if(next) {
					switch(next->stageType) {
						case STAGE_GLOBAL:
							throw std::string("qgl::RenderStageComposer: Cannot run STAGE_GLOBAL after STAGE_PER_CAMERA_FBO");
							break;
						case STAGE_PER_CAMERA:
							throw std::string("qgl::RenderStageComposer: Cannot run STAGE_PER_CAMERA after STAGE_PER_CAMERA_FBO");
							break;
						case STAGE_PER_CAMERA_FBO:
							currentStagesPerFbo.emplace_back(next, camera);
							break;
					}
				} else {
					auto c = camerasRenderingOrder[camera];
					if(c) {
						auto s = firstStageForCameraPerPipeline[stage->pipeline];
						switch(s->stageType) {
							case STAGE_GLOBAL:
								throw std::string("qgl::RenderStageComposer: Cannot run STAGE_GLOBAL for new camera");
								break;
							case STAGE_PER_CAMERA:
								currentStagesPerCamera.emplace_back(s, c);
								break;
							case STAGE_PER_CAMERA_FBO:
								currentStagesPerFbo.emplace_back(s, c);
								break;
						}
					}
				}
				auto end = std::chrono::steady_clock::now();
				uint64_t t = (end-begin).count();
				timings.push_back({stage->pipeline, stage, camera, t});
			}
		}
	}
	
	bool RenderStageComposer::HasAnyStagesLeft() {
		return currentStagesGlobal.size() || currentStagesPerCamera.size() ||
			currentStagesPerFbo.size();
	}
}

