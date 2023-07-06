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

#ifndef QUICKGL_LOG_HPP
#define QUICKGL_LOG_HPP

// #define QUICKGL_ENABLE_LOG

namespace qgl {
	class Log final {
	public:
		
		Log(int line, const char* fileName, const char* fmt, ...);
		
		static void EmptyLine(int emptyLines);
		
		static bool sync;
		
	private:
		
		inline const static char* LOG_FILE = "quickgl.log";
		
		static void LockStart();
		static void LockEnd();
		
		static void* GetFile();
		static double GetSecFromStart();
		static double GetSecFromLast();
	};
}

#ifdef QUICKGL_ENABLE_LOG
#define QUICKGL_LOG(...) {qgl::Log __lg(__LINE__, __FILE__, __VA_ARGS__);}
#else
#define QUICKGL_LOG(...) {}
#endif

#endif

