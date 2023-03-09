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

#ifndef QUICKGL_PIPELINE_IDS_MANAGED_BASE_HPP
#define QUICKGL_PIPELINE_IDS_MANAGED_BASE_HPP

#include "../util/IdsManager.hpp"

#include "Pipeline.hpp"

namespace qgl {
	class PipelineIdsManagedBase : public Pipeline {
	public:
		
		PipelineIdsManagedBase();
		virtual ~PipelineIdsManagedBase() = default;
		
		virtual uint32_t CreateEntity() final;
		virtual void DeleteEntity(uint32_t entityId) final;
		
	protected:
		
		virtual void FlushMeshManagerStateChangesToGPU() override;
		virtual void FlushDataToGPU() override;
		
	protected:

		qgl::IdsManager idsmanager;
		bool idsWereChanged;
		gl::VBO* idsBuffer;
	};
}

#endif

