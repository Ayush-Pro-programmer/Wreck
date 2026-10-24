#include <algorithm>
#include <cstdio>

#include "ConsoleLayer.h"
#include "Application.h"
#include "raylib.h"

namespace Wreck {

	ConsoleLayer* ConsoleLayer::s_Instance = nullptr;

	static void RaylibLogCallback(int logType, const char* text, va_list args)
	{
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), text, args);

		// add prefix tags based on log levels
		std::string prefix = "";
		FILE* outStream = stdout;

		switch (logType)
		{
		case LOG_INFO:		
			prefix = "INFO: ";
			break;
		case LOG_ERROR:		
			prefix = "ERROR: ";
			outStream = stderr;
			break;
		case LOG_WARNING:	
			prefix = "WARNING: ";
			outStream = stderr;
			break;
		case LOG_DEBUG:		
			prefix = "DEBUG: "; 
			break;
		default:			
			prefix = "LOG: ";
			break;
		}

		std::string finalMsg = prefix + std::string(buffer);
		fprintf(outStream, "%s\n", finalMsg.c_str());

		if (ConsoleLayer::Get())
			ConsoleLayer::Get()->Print(prefix + std::string(buffer));
	}

	ConsoleLayer::ConsoleLayer()
		: Layer("ConsoleLayer")
	{
		s_Instance = this;

		SetTraceLogCallback(RaylibLogCallback);

		Print("Wreck Engine v0.1 - Dev Console Initiaized.");
		Print("Type 'help' for a list of available commands.");
	}

	void ConsoleLayer::OnAttach()
	{
		m_CommandRegistry["help"] = [this](const auto& args) {
			Print("Available Commands:");
			Print("	map <map_name> - Load a WDF map file");
			Print("	clear          - Clear console history");
			Print("	exit           - Quit engine");
		};

		m_CommandRegistry["clear"] = [this](const auto& args) {
			m_History.clear();
		};

		m_CommandRegistry["exit"] = [this](const auto& args) {
			Wreck::Application::Get().Exit();
		};

		m_CommandRegistry["map"] = [this](const auto& args) {
			Print("WIP");
		};
	}

	void ConsoleLayer::OnUpdate(float dt)
	{
		if (IsKeyPressed(KEY_GRAVE)) // '`'
		{
			m_IsOpen = !m_IsOpen;
			if (m_IsOpen)
				EnableCursor();
			else
				DisableCursor();
		}

		float slideSpeed = 4.0f;
		if (m_IsOpen)
		{
			m_AnimProgress += slideSpeed * dt;
			if (m_AnimProgress > 1.0f) m_AnimProgress = 1.0f;
		}
		else
		{
			m_AnimProgress -= slideSpeed * dt;
			if (m_AnimProgress < 0.0f) m_AnimProgress = 0.0f;
		}

		if (!m_IsOpen && m_AnimProgress <= 0.0f) return;

		if (m_IsOpen)
		{
			// handle text input typing
			int key = GetCharPressed();
			while (key > 0)
			{
				// add printable characters to buffer
				if (key != 96 && (key >= 32) && (key <= 125) && (m_InputLen < sizeof(m_InputBuffer) - 1))
				{
					m_InputBuffer[m_InputLen] = (char)key;
					m_InputBuffer[m_InputLen + 1] = '\0';
					m_InputLen++;
				}
				key = GetCharPressed();
			}

			// handle backspace
			if (IsKeyPressed(KEY_BACKSPACE) && m_InputLen > 0)
			{
				m_InputLen--;
				m_InputBuffer[m_InputLen] = '\0';
			}

			// handle command execution on ENTER
			if (IsKeyPressed(KEY_ENTER) && m_InputLen > 0)
			{
				std::string command(m_InputBuffer);
				Print("> " + command);
				ExecuteCommand(command);

				// clear input buffer
				memset(m_InputBuffer, 0, sizeof(m_InputBuffer));
				m_InputLen = 0;
			}
		}
	}

	void ConsoleLayer::OnRender()
	{
		if (m_AnimProgress <= 0.0f) return;

		int screenWidth = GetScreenWidth();
		int screenHeight = GetScreenHeight();
		float maxConsoleHeight = screenHeight * 0.5f;

		int currentConsoleHeight = static_cast<int>(maxConsoleHeight * m_AnimProgress);
		currentConsoleHeight = (currentConsoleHeight / 16) * 16;

		DrawRectangle(0, 0, screenWidth, currentConsoleHeight, Fade(BLACK, 0.85));
		DrawRectangleLines(0, 0, screenWidth, currentConsoleHeight, DARKGRAY);

		if (m_AnimProgress > 0.4f)
		{
			int consoleHeight = static_cast<int>(currentConsoleHeight);
			int fontSize = 18;
			int padding = 10;
			int yPos = consoleHeight - fontSize - padding - 20; // leaving room for input line

			for (auto it = m_History.rbegin(); it != m_History.rend(); it++)
			{
				if (yPos < padding) break;
				DrawText(it->c_str(), padding, yPos, fontSize, LIGHTGRAY);
				yPos -= (fontSize + 4);
			}

			DrawLine(padding, consoleHeight - 30, screenWidth - padding, consoleHeight - 30, GREEN);

			std::string prompt = "] " + std::string(m_InputBuffer);
			DrawText(prompt.c_str(), padding, consoleHeight - 22, fontSize, WHITE);

			static float cursorTimer = 0.0f;
			cursorTimer += GetFrameTime();
			if ((int)(cursorTimer * 2) % 2 == 0)
			{
				int texWidth = MeasureText(prompt.c_str(), fontSize);
				DrawRectangle(padding + texWidth + 4, consoleHeight - 22, 8, fontSize, GREEN);
			}
		}
	}

	void ConsoleLayer::Print(const std::string& text)
	{
		m_History.push_back(text);
		
		// keeping history capped at 100 lines
		if (m_History.size() > 100)
			m_History.erase(m_History.begin());
	}

	void ConsoleLayer::ExecuteCommand(const std::string& cmdLine)
	{
		std::stringstream ss(cmdLine);
		std::string command;
		ss >> command;
		std::transform(command.begin(), command.end(), command.begin(), ::tolower);

		// parse extra args
		std::vector<std::string> args;
		std::string arg;
		while (ss >> arg) args.push_back(arg);

		// look up and execute
		auto it = m_CommandRegistry.find(command);
		if (it != m_CommandRegistry.end())
			it->second(args);
		else
			Print("Unknown command: '" + command + "'. Type 'help' for options.");
	}
}