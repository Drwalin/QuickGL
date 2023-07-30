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

#include "../../include/quickgl/resources/Resource.hpp"

namespace qgl {
		
	Resource::Resource(const std::string& name, const std::string& path) :
		name(name), path(path) {
		referenceCounter = 0;
		loaded = false;
	}
	
	Resource::~Resource() {
		Destroy();
	}
	
	void Resource::Destroy() {
		if(loaded) {
			Unload();
			loaded = 0;
			referenceCounter = 0;
		}
	}
	
	void Resource::AddReference() {
		++referenceCounter;
		if(!loaded) {
			Load();
		}
	}
	
	void Resource::RemoveReference() {
		--referenceCounter;
		if(loaded && referenceCounter == 0) {
			Unload();
			loaded = false;
		}
	}
		
	uint64_t Resource::GetMemoryConsumption() const {
		return sizeof(*this) + name.capacity() + path.capacity();
	}
}

