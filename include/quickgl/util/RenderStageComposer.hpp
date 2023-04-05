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

#include "../pipelines/Pipeline.hpp"

#include <list>
#include <thread>

namespace qgl {
	
	struct Timings {
		uint32_t stage;
		uint32_t pipeline;
		uint64_t seconds_queue;
		std::string name;
	};
	
	class RenderStageComposer final {
	public:
		
		void AddPipelineStages(std::shared_ptr<Pipeline> pipeline);
		
		uint32_t RestartStages();
		uint32_t NextStage(std::shared_ptr<Camera> camera);
		
		const std::vector<Timings>& GetTimings() const { return timings; }
		
	private:
		
		std::vector<Timings> timings;
		
		std::list<std::vector<Pipeline::StageFunction>> currentStagingFunctions;
		uint32_t currentStage;
		
		std::vector<std::vector<Pipeline::StageFunction>> stagingFunctions;
	};
}

#endif


