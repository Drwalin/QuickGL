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

#ifndef QUICKGL_RENDER_STAGE_COMPOSER_HPP
#define QUICKGL_RENDER_STAGE_COMPOSER_HPP

#include <cinttypes>

#include <unordered_map>
#include <string>
#include <chrono>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <functional>

namespace qgl {
	class Camera;
	class Pipeline;
	
	enum StageTypeFlags : uint32_t {
		/*
		 * Waits for aother materials/pipelines to finish all stage before stageId
		 * for current camera.
		 */
		STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA = 1,
		STAGE_REQUIRE_BOUND_FBO = 2,
	};

	enum StageOrder : uint32_t {
		STAGE_UPDATE_DATA = 0,
		STAGE_GLOBAL = 2,
		STAGE_CAMERA = 4,
		
		STAGE_1_RENDER_PASS_1 = 8 | STAGE_REQUIRE_BOUND_FBO,
		STAGE_2_OCCLUSION_PASS_1 = 8 | STAGE_REQUIRE_BOUND_FBO | STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA,
		STAGE_3_RENDER_PASS_2 = 12 | STAGE_REQUIRE_BOUND_FBO,
		STAGE_4_OCCLUSION_PASS_2 = 12 | STAGE_REQUIRE_BOUND_FBO | STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA,
		STAGE_5_RENDER_PASS_3 = 16 | STAGE_REQUIRE_BOUND_FBO,
		
		STAGE_RENDER_PASS_WATER = 64 | STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA,
		
		STAGE_RENDER_PASS_TRANSLUCENT = 128 | STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA,
		
		STAGE_POST_PROCESS = 256 | STAGE_SYNC_AFTER_OTHER_MATERIALS_CURRENT_CAMERA,
	};
	
	struct Stage {
		Stage(Stage&&) = default;
		Stage(Stage&) = delete;
		Stage(const Stage&) = delete;
		Stage& operator=(Stage&&) = delete; // default;
		Stage& operator=(Stage&) = delete;
		Stage& operator=(const Stage&) = delete;
		
		void Execute(std::shared_ptr<Camera> camera);
		bool CanExecute(std::shared_ptr<Camera> camera);
		
		inline Stage(
				std::string name,
				StageOrder stageOrder,
				std::shared_ptr<Pipeline> pipeline,
				void(Pipeline::* taskFunction)(std::shared_ptr<Camera>),
				bool(Pipeline::* canExecute)(std::shared_ptr<Camera>) = nullptr) :
					name(name),
					executionPolicy(stageOrder),
					taskFunction(taskFunction),
					canExecute(canExecute),
					pipeline(pipeline) {
		}
		
		const std::string name;
		const StageOrder executionPolicy;
		void(Pipeline::* taskFunction)(std::shared_ptr<Camera>);
		bool(Pipeline::* canExecute)(std::shared_ptr<Camera>);
		const std::shared_ptr<Pipeline> pipeline;
	};
	
	struct StageTiming {
		StageTiming() = default;
		StageTiming(StageTiming&&) = default;
		StageTiming(StageTiming&) = default;
		StageTiming(const StageTiming&) = default;
		StageTiming& operator=(StageTiming&&) = default;
		StageTiming& operator=(StageTiming&) = default;
		StageTiming& operator=(const StageTiming&) = default;
		
		std::shared_ptr<struct Stage> stage;
		std::shared_ptr<Camera> camera;
		double measuredSeconds;
		decltype(std::chrono::steady_clock::now()) start;
		
		void Start(std::shared_ptr<Stage> stage);
		void End();
	};
	
	class PipelineStagesScheduler {
	public:
		
		void Init(std::shared_ptr<Pipeline> pipeline);
		void Destroy();
		
		void AddStage(std::shared_ptr<Stage> stage);
		template<typename T>
		void AddStage(
				std::string name,
				StageOrder stageOrder,
				void(T::* taskFunction)(std::shared_ptr<Camera>),
				bool(T::* canExecute)(std::shared_ptr<Camera>) = nullptr) {
			AddStage(std::make_shared<Stage>(name, stageOrder, pipeline,
						(void(Pipeline::*)(std::shared_ptr<Camera>))taskFunction,
						(bool(Pipeline::*)(std::shared_ptr<Camera>))canExecute));
		}
		
		bool HasMoreStages();
		bool CanExecuteNextStage();
		void ExecuteNextStage();
		
		void RestartExecution(
				class RenderStageComposer* renderStageComposer);
		
		std::shared_ptr<Stage> GetNextStage();
		
		std::shared_ptr<Camera> GetCurrentCamera() { return currentCamera; }
		
	public:
		
		uint32_t nextGlobalStage;
		uint32_t nextPerCameraStage;
		
		std::shared_ptr<Camera> currentCamera;
		uint32_t currentCameraId;
		
		std::vector<std::shared_ptr<Stage>> globalStages;
		std::vector<std::shared_ptr<Stage>> perCameraStages;
		
		RenderStageComposer* renderStageComposer;
		
		std::shared_ptr<Pipeline> pipeline;
	};
	
	class RenderStageComposer final {
	public:
		
		RenderStageComposer();
		
		void Destroy();
		
		void AddPipeline(std::shared_ptr<Pipeline> pipeline);
		void AddCamera(std::shared_ptr<Camera> camera);
		void RemoveCamera(std::shared_ptr<Camera> camera);
		
		void RenderAsLast(std::shared_ptr<Camera> camera);
		
		void ResetExecution();
		
		/* 
		 * return false when nothing was updated (no ::canBeContinue() returned
		 * true.
		 */
		bool ContinueStages();
		bool HasAnyStagesLeft();
		
		bool CanExecuteSyncStage(uint32_t cameraId, StageOrder stageOrder);
		
		std::shared_ptr<Camera> GetCameraByIndex(uint32_t id);
		
		void SetGlFinishInEveryStageToProfile(bool value);
		
		std::vector<StageTiming> GetTimings() const;
		double GetTotalCpuTime() const;
		
	private:
		
		std::shared_ptr<Camera> lastRenderCamera;
		
		bool enableGlFinishInEveryStageToProfile;
		
		std::vector<StageTiming> timings;
		double totalCpuTime;
		
		std::map<uint32_t,
			std::map<std::shared_ptr<Pipeline>,
				StageOrder>> mapCurrentCameraIdToPipelines;
		
		std::vector<std::shared_ptr<Camera>> cameras;
		std::vector<std::shared_ptr<Pipeline>> pipelines;
		
		bool hasAnyStagesLeft;
	};
}

#endif

