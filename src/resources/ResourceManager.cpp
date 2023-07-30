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

#include "../../include/quickgl/resources/ResourceManager.hpp"
#include "../../include/quickgl/resources/Resource.hpp"
#include "../../include/quickgl/resources/ResourceType.hpp"

namespace qgl {
	
	ResourceManager::ResourceManager() {
	}
	
	ResourceManager::~ResourceManager() {
	}
		
	void ResourceManager::RegisterResourceType(
			std::shared_ptr<ResourceType> resourceType) {
		resourceTypes[resourceType->GetResourceTypeName()] = resourceType;
	}
	
	void ResourceManager::RegisterResource(const std::string& name,
			std::string path, ...) {
		std::string resourceType = name.substr(0, name.find('.'));
		auto type = resourceTypes.find(resourceType);
		if(type == resourceTypes.end())
			throw std::string("Trying to load register unkown resource type: ") + resourceType;
		va_list varArgs;
		va_start(varArgs, path);
		type->second->RegisterResource(name, path, varArgs);
		va_end(varArgs);
	}
	
	std::shared_ptr<Resource> ResourceManager::GetResource(
			const std::string& name) {
		auto it = resources.find(name);
		if(it == resources.end())
			return nullptr;
		return it->second;
	}
}

