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

#ifndef QUICKGL_RESOURCE_TYPE_HPP
#define QUICKGL_RESOURCE_TYPE_HPP

#include <cstdarg>

#include <memory>
#include <string>

namespace qgl {
	class Resource;
	
	class ResourceType : public std::enable_shared_from_this<Resource> {
	public:
		
		ResourceType(const std::string& resourceTypeName);
		virtual ~ResourceType();
		
		virtual void RegisterResource(const std::string& name,
				const std::string& path, va_list varArgs) = 0;
		
		inline const std::string& GetResourceTypeName() const { return resourceTypeName; }
		
	private:
		
		const std::string resourceTypeName;
	};
}

#endif


