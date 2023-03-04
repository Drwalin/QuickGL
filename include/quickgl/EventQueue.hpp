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

#ifndef QUICKGL_EVENT_QUEUE_HPP
#define QUICKGL_EVENT_QUEUE_HPP

#include <functional>
#include <atomic>
#include <queue>
#include <mutex>

namespace qgl {
	class EventQueue {
	public:
		
		EventQueue() = default;
		~EventQueue() = default;
		
		template<typename... Args, typename Fn>
		void PushEvent(Fn event, Args... args) {
			PushEvent_(std::bind(event, args...));
		}
		
		void PushEvent_(std::function<void()> event);
		
		bool ExecuteOneEvent(); // returns true if anything was executed
		bool HasAnyEvents();
		
	private:
		
		std::atomic<uint32_t> counter;
		
		// replace std::function & std::bind with something faster/smaller
		std::queue<std::function<void()>> eventsQueue;
		
		// replace std::queue & std::mutex with something faster
		std::mutex mutex;
		
		std::queue<std::function<void()>> popedEventsForRunningThreads;
	};
}

#endif

