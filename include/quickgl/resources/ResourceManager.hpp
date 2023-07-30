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

#ifndef QUICKGL_RESOURCE_MANAGER_HPP
#define QUICKGL_RESOURCE_MANAGER_HPP

#include <memory>
#include <string>
#include <unordered_map>

namespace qgl {
	class ResourceType;
	class Resource;
	
	/*
	 * Resources should be named in a fashion:
	 *     type.name.separated.by.dots
	 * For example:
	 *     texture.icons.infernal_sword_1
	 *     texture.terrain_heightmap.mainland
	 *     texture.main_menu_background
	 *     mesh_static.tree
	 *     mesh_bone_animated.goblin
	 *     bone_animation.swinging_sword
	 */
	
	class ResourceManager final : public std::enable_shared_from_this<ResourceManager> {
	public:
		
		ResourceManager();
		~ResourceManager();
		
		void RegisterResourceType(std::shared_ptr<ResourceType> resourceType);
		
		void RegisterResource(const std::string& name,
				std::string path, ...);
		
		std::shared_ptr<Resource> GetResource(const std::string& name);
		
	private:
		
		std::unordered_map<std::string, std::shared_ptr<Resource>> resources;
		std::unordered_map<std::string, std::shared_ptr<ResourceType>> resourceTypes;
	};
}

#endif

