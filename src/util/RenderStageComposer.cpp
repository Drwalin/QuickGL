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
#include "../../include/quickgl/pipelines/Pipeline.hpp"

#include "../../include/quickgl/util/RenderStageComposer.hpp"

namespace qgl {
	void StageTiming::Start(std::shared_ptr<Stage> stage) {
		this->stage = stage;
		camera = stage->pipeline->GetStageScheduler().GetCurrentCamera();
		start = std::chrono::steady_clock::now();
	}
	
	void StageTiming::End() {
		auto end = std::chrono::steady_clock::now();
		measuredSeconds =
				std::chrono::duration_cast<
					std::chrono::duration<double>>(
							end - start).count();
	}
	
	
	
	void Stage::Execute(std::shared_ptr<Camera> camera) {
		(pipeline.get()->*(taskFunction))(camera);
	}
	
	bool Stage::CanExecute(std::shared_ptr<Camera> camera) {
		if(canExecute)
			return (pipeline.get()->*(canExecute))(camera);
		return true;
	}
	
	
	
	void PipelineStagesScheduler::Init(std::shared_ptr<Pipeline> pipeline) {
		this->pipeline = pipeline;
	}
	
	void PipelineStagesScheduler::Destroy() {
		currentCamera = nullptr;
		globalStages.clear();
		perCameraStages.clear();
		renderStageComposer = nullptr;
		pipeline = nullptr;
	}
	
	void PipelineStagesScheduler::AddStage(std::shared_ptr<Stage> stage) {
		if(stage->executionPolicy < STAGE_CAMERA) {
			for(int i=0; i<globalStages.size(); ++i) {
				if(globalStages[i]->executionPolicy > stage->executionPolicy) {
					globalStages.insert(globalStages.begin() + 1, stage);
					return;
				}
			}
			globalStages.push_back(stage);
		} else {
			for(int i=0; i<perCameraStages.size(); ++i) {
				if(perCameraStages[i]->executionPolicy > stage->executionPolicy) {
					perCameraStages.insert(perCameraStages.begin() + 1, stage);
					return;
				}
			}
			perCameraStages.push_back(stage);
		}
	}
	
	bool PipelineStagesScheduler::HasMoreStages() {
		return GetNextStage() != nullptr;
	}
	
	bool PipelineStagesScheduler::CanExecuteNextStage() {
		auto stage = GetNextStage();
		bool ret = stage->CanExecute(currentCamera);
		if(currentCamera && ret
				&& stage->executionPolicy & STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA) {
			return renderStageComposer->CanExecuteSyncStage(currentCameraId,
					stage->executionPolicy);
		}
		return ret;
	}
	
	void PipelineStagesScheduler::ExecuteNextStage() {
		if(HasMoreStages() && CanExecuteNextStage()) {
			auto stage = GetNextStage();
			if(stage->executionPolicy & STAGE_REQUIRE_BOUND_FBO
					&& stage->pipeline->GetStageScheduler().GetCurrentCamera()) {
				stage->pipeline->GetStageScheduler().GetCurrentCamera()->UseFbo();
			}
			stage->Execute(currentCamera);
			
			if(nextGlobalStage < globalStages.size()) {
				++nextGlobalStage;
				if(nextGlobalStage >= globalStages.size()) {
					nextPerCameraStage = 0;
					currentCamera = renderStageComposer
						->GetCameraByIndex(currentCameraId);
					if(currentCamera == nullptr) {
						nextPerCameraStage = perCameraStages.size();
					}
				}
			} else if(nextPerCameraStage < perCameraStages.size()) {
				++nextPerCameraStage;
				if(nextPerCameraStage >= perCameraStages.size()) {
					++currentCameraId;
					nextPerCameraStage = 0;
					currentCamera = renderStageComposer
						->GetCameraByIndex(currentCameraId);
					if(currentCamera == nullptr) {
						nextPerCameraStage = perCameraStages.size();
					}
				}
			}
		}
	}
	
	void PipelineStagesScheduler::RestartExecution(
			RenderStageComposer* renderStageComposer) {
		this->renderStageComposer = renderStageComposer;
		nextGlobalStage = 0;
		nextPerCameraStage = 0;
		currentCameraId = 0;
		currentCamera = nullptr;
	}
	
	std::shared_ptr<Stage> PipelineStagesScheduler::GetNextStage() {
		if(nextGlobalStage < globalStages.size()) {
			return globalStages[nextGlobalStage];
		} else if(nextPerCameraStage < perCameraStages.size()) {
			return perCameraStages[nextPerCameraStage];
		}
		return nullptr;
	}
	

	
	
	
	
	RenderStageComposer::RenderStageComposer() {
		enableGlFinishInEveryStageToProfile = false;
	}
	
	void RenderStageComposer::AddPipeline(std::shared_ptr<Pipeline> pipeline) {
		pipelines.push_back(pipeline);
	}
	
	void RenderStageComposer::AddCamera(std::shared_ptr<Camera> camera) {
		cameras.push_back(camera);
		RenderAsLast(lastRenderCamera);
	}
	
	void RenderStageComposer::RemoveCamera(std::shared_ptr<Camera> camera) {
		for(int i=0; i<cameras.size(); ++i) {
			if(cameras[i] == camera) {
				cameras.erase(cameras.begin()+1);
				return;
			}
		}
	}
	
	void RenderStageComposer::RenderAsLast(std::shared_ptr<Camera> camera) {
		lastRenderCamera = camera;
		if(camera != nullptr) {
			RemoveCamera(camera);
			cameras.push_back(camera);
		}
	}
	
	void RenderStageComposer::ResetExecution() {
		auto start = std::chrono::steady_clock::now();
		timings.clear();
		RenderAsLast(lastRenderCamera);
		for(auto p : pipelines) {
			p->GetStageScheduler().RestartExecution(this);
		}
		hasAnyStagesLeft = true;
		auto end = std::chrono::steady_clock::now();
		totalCpuTime =
				std::chrono::duration_cast<
					std::chrono::duration<double>>(
							end - start).count();
	}
	
	bool RenderStageComposer::ContinueStages() {
		auto start = std::chrono::steady_clock::now();
		if(this->enableGlFinishInEveryStageToProfile) {
			gl::Finish();
		}
		hasAnyStagesLeft = false;
		bool executedAny = false;
		for(auto p : pipelines) {
			PipelineStagesScheduler& s = p->GetStageScheduler();
			if(s.HasMoreStages()) {
				hasAnyStagesLeft = true;
				if(s.CanExecuteNextStage()) {
					timings.emplace_back();
					auto stage = s.GetNextStage();
					timings.back().Start(stage);
					s.ExecuteNextStage();
					executedAny = true;
					if(this->enableGlFinishInEveryStageToProfile) {
						gl::Finish();
					}
					hasAnyStagesLeft |= s.HasMoreStages();
					timings.back().End();
				}
			}
		}
		gl::Flush();
		auto end = std::chrono::steady_clock::now();
		totalCpuTime +=
				std::chrono::duration_cast<
					std::chrono::duration<double>>(
							end - start).count();
		return executedAny;
	}
	
	bool RenderStageComposer::HasAnyStagesLeft() {
		return hasAnyStagesLeft;
	}
	
	bool RenderStageComposer::CanExecuteSyncStage(uint32_t cameraId,
			StageOrder stage) {
		for(auto p : pipelines) {
			auto s = p->GetStageScheduler().GetNextStage();
			if(s) {
				if(s->executionPolicy < STAGE_CAMERA) {
					return false;
				}
				if(p->GetStageScheduler().currentCameraId < cameraId) {
					return false;
				} else if(p->GetStageScheduler().currentCameraId == cameraId) {
					if(s->executionPolicy < stage) {
						return false;
					}
				}
			}
		}
		return true;
	}
	
	std::shared_ptr<Camera> RenderStageComposer::GetCameraByIndex(uint32_t id) {
		if(id < cameras.size()) {
			return cameras[id];
		}
		return nullptr;
	}
	
	void RenderStageComposer::SetGlFinishInEveryStageToProfile(bool value) {
		enableGlFinishInEveryStageToProfile = value;
	}
	
	std::vector<StageTiming> RenderStageComposer::GetTimings() const {
		return timings;
	}
	
	double RenderStageComposer::GetTotalCpuTime() const {
		return totalCpuTime;
	}
		
	void RenderStageComposer::Destroy() {
		lastRenderCamera = nullptr;
		timings.clear();
		
		mapCurrentCameraIdToPipelines.clear();
		
		cameras.clear();
		pipelines.clear();
	}
}

