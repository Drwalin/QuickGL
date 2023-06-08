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

#include <unordered_map>
#include <string>
#include <vector>

#include "../pipelines/Pipeline.hpp"

namespace qgl {
	
	struct Timings {
		std::shared_ptr<Pipeline> pipeline;
		std::shared_ptr<struct Stage> stage;
		std::shared_ptr<Camera> camera;
		uint64_t measuredTimeNanoseconds;
	};
	
	enum StageType {
		// any next stage cannot have lower stage type then current
		STAGE_GLOBAL = 1,
		STAGE_PER_CAMERA = 2,
		STAGE_PER_CAMERA_FBO = 3,
	};
	
	struct Stage {
		Stage() = default;
		Stage(Stage&&) = default;
		Stage(Stage&) = default;
		Stage(const Stage&) = default;
		Stage& operator=(Stage&&) = default;
		Stage& operator=(Stage&) = default;
		Stage& operator=(const Stage&) = default;
		Stage(std::string name, StageType t,
				Pipeline::StageFunction renderFunction,
				std::function<bool(std::shared_ptr<Camera>)> canBeContinued=nullptr) :
				stageName(name), stageType(t), renderFunction(renderFunction),
				canBeContinued(canBeContinued) {
		}
		
		std::string stageName;
		StageType stageType;
		Pipeline::StageFunction renderFunction;
		std::function<bool(std::shared_ptr<Camera>)> canBeContinued;
		std::shared_ptr<Stage> nextStage;
		std::shared_ptr<Pipeline> pipeline;
	};
	
	struct StageCameraPair {
		StageCameraPair() = default;
		StageCameraPair(StageCameraPair&&) = default;
		StageCameraPair(StageCameraPair&) = default;
		StageCameraPair(const StageCameraPair&) = default;
		StageCameraPair& operator=(StageCameraPair&&) = default;
		StageCameraPair& operator=(StageCameraPair&) = default;
		StageCameraPair& operator=(const StageCameraPair&) = default;
		StageCameraPair(std::shared_ptr<Stage> stage,
				std::shared_ptr<Camera> camera) :
				stage(stage), camera(camera) {
		}
		std::shared_ptr<Stage> stage;
		std::shared_ptr<Camera> camera;
	};
	
	class RenderStageComposer final {
	public:
		
		void AddPipelineStages(std::shared_ptr<Pipeline> pipeline);
		
		void RestartStages(std::vector<std::shared_ptr<Camera>> cameras);
		
		void ContinueStages();
		bool HasAnyStagesLeft();
		
		void Clear();
		
		inline const std::vector<Timings>& GetTimings() const { return timings; }
		inline uint64_t CountNanosecondsOnCpu() const { return sumTimeCount; }
		inline uint64_t CountTotalNanosecondsOnCpu() const { return totalTimeCountNs; }
		void PrintStagesStructure();
		
	private:
		
		void ContinueStagesGlobal();
		void ContinueStagesPerCamera();
		void ContinueStagesPerFbo();
		
	private:
		
		uint64_t sumTimeCount;
		uint64_t totalTimeCountNs;
		std::vector<Timings> timings;
		
		std::unordered_map<std::shared_ptr<Camera>,
			std::shared_ptr<Camera>> camerasRenderingOrder;
		
		std::vector<std::shared_ptr<Stage>> currentStagesGlobal;
		std::vector<StageCameraPair> currentStagesPerCamera;
		std::vector<StageCameraPair> currentStagesPerFbo;
		
		std::unordered_map<std::shared_ptr<Pipeline>,
			std::shared_ptr<Stage>> firstStageForCameraPerPipeline;
		
		std::vector<std::shared_ptr<Stage>> stagingPipelineTemplate;
	};
}

#endif

