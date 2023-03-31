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

#include "../../include/quickgl/util/RenderStageComposer.hpp"

namespace qgl {
	
	void RenderStageComposer::AddPipelineStages(
			std::shared_ptr<Pipeline> pipeline) {
		stagingFunctions.emplace_back();
		pipeline->AppendRenderStages(stagingFunctions.back());
	}
	
	uint32_t RenderStageComposer::RestartStages() {
		currentStagingFunctions = std::list(stagingFunctions.begin(), stagingFunctions.end());
		currentStage = 0;
		uint32_t max = 0;
		for(const auto& e : currentStagingFunctions) {
			max = std::max<uint32_t>(e.size(), max);
		}
		return max;
	}
	
	uint32_t RenderStageComposer::NextStage(std::shared_ptr<Camera> camera) {
		std::list<std::vector<Pipeline::StageFunction>> copy;
		for(auto& e : currentStagingFunctions) {
			e[currentStage](camera);
			if(currentStage+1 < e.size()) {
				copy.emplace_back();
				copy.back().swap(e);
			}
		}
		currentStagingFunctions.swap(copy);
		++currentStage;
		return copy.size()>0 ? 1 : 0;
	}
}

