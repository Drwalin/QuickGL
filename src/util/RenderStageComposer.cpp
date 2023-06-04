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

#include "../../OpenGLWrapper/include/openglwrapper/OpenGL.hpp"

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
		timings.clear();
		return max;
	}
	
	void RenderStageComposer::Clear() {
		timings.clear();
		currentStagingFunctions.clear();
		stagingFunctions.clear();
	}
	
	uint32_t RenderStageComposer::NextStage(std::shared_ptr<Camera> camera) {
		std::list<std::vector<Pipeline::StageFunction>> copy;
		uint32_t i=0;
		for(auto& e : currentStagingFunctions) {
			auto s = std::chrono::steady_clock::now();
			e[currentStage](camera);
			gl::Finish();
			auto e1 = std::chrono::steady_clock::now();
			uint64_t t1 = (e1-s).count();
			timings.push_back({currentStage, i, t1});
			switch(currentStage) {
				case 0:
					timings.back().name = "update data copy delta";
					break;
				case 1:
					timings.back().name = "update data compute shader update";
					break;
				case 2:
					timings.back().name = "update camera clipping planes";
					break;
				case 3:
					timings.back().name = "generate frustum culled entity ids buffer";
					break;
				case 4:
					timings.back().name = "generate indirect buffer";
					break;
				case 5:
					timings.back().name = "fetch frustum culled entities count";
					break;
				case 6:
					timings.back().name = "multi draw indirect";
					break;
			}
			if(currentStage+1 < e.size()) {
				copy.emplace_back();
				copy.back().swap(e);
			}
			++i;
		}
		
		gl::Flush();
		
		currentStagingFunctions.swap(copy);
		++currentStage;
		return copy.size()>0 ? 1 : 0;
	}
}

