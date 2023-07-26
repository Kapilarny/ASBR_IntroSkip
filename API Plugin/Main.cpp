#include "Main.h"

#include <stdio.h>
#include <vector>
#include <iostream>
#include <filesystem>
#include <fstream>

void PatchEx(BYTE* dst, BYTE* src, unsigned int size, HANDLE hProcess)
{
	DWORD oldprotect;
	VirtualProtectEx(hProcess, dst, size, PAGE_EXECUTE_READWRITE, &oldprotect);
	if (!WriteProcessMemory(hProcess, dst, src, size, nullptr)) {
		printf("[IntroSkip]: Failed to write to memory!\n");

		// Print the error code.
		printf("[IntroSkip]: Error code: %d\n", GetLastError());

		return;
	}
	VirtualProtectEx(hProcess, dst, size, oldprotect, &oldprotect);
}

void WaitAndGetWindow() {
	plugin::windowHandle = FindWindowA(NULL, "Jojo's Bizarre Adventure: All-Star Battle R");
	if (!plugin::windowHandle) { // If the window handle is null, it means that the game didnt initialize the window yet. Wait 10ms and try again until it we get the handle.
		Sleep(10); 
		WaitAndGetWindow();
	}
}

// This function is called when booting the game.
// In the modding api, 0xC00 is added to the module base by default. In my modified code, I am removing it.
void InitializePlugin(__int64 a, std::vector<__int64> b)
{
	plugin::moduleBase = a - 0xC00;

	// Get the window handle and process id.
	WaitAndGetWindow();

	GetWindowThreadProcessId(plugin::windowHandle, &plugin::processId);

	if (!plugin::processId) {
		printf("[IntroSkip]: Failed to get process id!\n");
		return;
	}

	// Get the process handle.
	plugin::processHandle = OpenProcess(PROCESS_ALL_ACCESS, true, plugin::processId);

	if (!plugin::processHandle) {
		printf("[IntroSkip]: Failed to get process handle!\n");
		return;
	}

	char* string = (char*)malloc(sizeof(char) * 14);
	memset(string, 0, sizeof(char) * 14);
	SIZE_T bytesRead = 0;

	// Read the string from memory.
	ReadProcessMemory(plugin::processHandle, (void*) (plugin::moduleBase + 0xE4DC38), string, 14, &bytesRead);
	
	if (!bytesRead) {
		printf("[IntroSkip]: Failed to read string!\n");
		return;
	}

	if (strcmp(string, "V_title_%s_03")) {
		printf("[IntroSkip]: Read string is not correct!\n");

		// Print the string.
		printf("[IntroSkip]: Read string: %s\n", string);

		return;
	}

	// Patch the string.
	char newString[] = "xxx";
	PatchEx((BYTE*)(plugin::moduleBase + 0xE4DC38), (BYTE*)newString, 3, plugin::processHandle);

	// Patch the function pointers.
	
	// Get the original function pointer
	BYTE* newBytes = (BYTE*)malloc(sizeof(BYTE) * 8);
	memset(newBytes, 0, sizeof(BYTE) * 8);
	ReadProcessMemory(plugin::processHandle, (void*)(plugin::moduleBase + 0xE5AB90), newBytes, 8, &bytesRead);

	// Override the function pointers.
	PatchEx((BYTE*)(plugin::moduleBase + 0xE5AB98), (BYTE*)newBytes, 8, plugin::processHandle);
	PatchEx((BYTE*)(plugin::moduleBase + 0xE5ABC8), (BYTE*)newBytes, 8, plugin::processHandle);

	printf("[IntroSkip]: Applied patches!\n");
}

// This function adds commands to the API's console.
void InitializeCommands(__int64 a, __int64 addCommandFunctionAddress) {}

// Use this function to hook any of the game's original functions.
void InitializeHooks(__int64 a, __int64 hookFunctionAddress) {}

// Use this function to add any lua commands to the game.
void InitializeLuaCommands(__int64 a, __int64 addCommandFunction) {}

// This function will be called all the time while you're playing after the plugin has been initialized.
void GameLoop(__int64 a) {}

// This function is called when the API is loading a mod's files. Return true if the file was read by this plugin, otherwise return false for the API to manage the file.
bool ParseApiFiles(__int64 a, std::string filePath, std::vector<char> fileBytes)
{
	return false;
}
