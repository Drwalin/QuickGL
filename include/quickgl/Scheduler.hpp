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

#ifndef QUICKGL_SCHEDULER_HPP
#define QUICKGL_SCHEDULER_HPP

#include "EventQueue.hpp"
#include "DelayedEvents.hpp"

namespace qgl {
	class Scheduler {
	public:
		
		Scheduler() = default;
		~Scheduler() = default;
		
		void Run();
		
		template<typename... Args>
		void ScheduleTask(std::function<void(Args...)> task, Args... args) {
			regularEvents.PushEvent(task, args...);
		}
		
		template<typename... Args>
		void ScheduleTask(void(*task)(Args...), Args... args) {
			regularEvents.PushEvent(task, args...);
		}
		
		
		template<typename... Args>
		void SchedulePriorityTask(std::function<void(Args...)> task,
				Args... args) {
			priorityEvents.PushEvent(task, args...);
		}
		
		template<typename... Args>
		void SchedulePriorityTask(void(*task)(Args...),
				Args... args) {
			priorityEvents.PushEvent(task, args...);
		}
		
		
		template<typename... Args>
		void ScheduleDelayedTask(int msDelay, std::function<void(Args...)> task,
				Args... args) {
			delayedEvents.PushEvent(msDelay, task, args...);
		}
		
		template<typename... Args>
		void ScheduleDelayedTask(int msDelay, void(*task)(Args...),
				Args... args) {
			delayedEvents.PushEvent(msDelay, task, args...);
		}
		
		bool ExecuteOne(); // returns true if anything was executed
		
	private:
		
		EventQueue regularEvents;
		EventQueue priorityEvents;
		
		DelayedEvents delayedEvents;
	};
}

#endif

