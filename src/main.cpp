#include "pch.h"
#include "dx_context.h"
#include "dx_window.h"
#include "dx_command_list.h"
#include "input.h"
#include "imgui.h"

dx_context dxContext;


static float perfFreq;
static LARGE_INTEGER lastTime;

bool handleWindowsMessages(user_input& input);

bool newFrame(user_input& input, float& dt)
{
	LARGE_INTEGER currentTime;
	QueryPerformanceCounter(&currentTime);
	dt = ((float)(currentTime.QuadPart - lastTime.QuadPart) / perfFreq);
	lastTime = currentTime;

	bool result = handleWindowsMessages(input);

	// Quit when escape is pressed, but not if in combination with ctrl or shift. This combination is usually pressed to open the task manager.
	if (buttonDownEvent(input, button_esc) && !(isDown(input, button_ctrl) || isDown(input, button_shift)))
	{
		result = false;
	}

	newImGuiFrame(input, dt);
	ImGui::DockSpaceOverViewport();

	return result;
}

static uint64 renderToWindow(dx_window& window, float* clearColor)
{
	dx_resource backbuffer = window.backBuffers[window.currentBackbufferIndex];
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(window.rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), window.currentBackbufferIndex, window.rtvDescriptorSize);


	dx_command_list* cl = dxContext.getFreeRenderCommandList();

	CD3DX12_RECT scissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
	CD3DX12_VIEWPORT viewport = CD3DX12_VIEWPORT(0.f, 0.f, (float)window.clientWidth, (float)window.clientHeight);

	cl->setScissor(scissorRect);
	cl->setViewport(viewport);

	cl->transitionBarrier(backbuffer, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_RENDER_TARGET);

	cl->clearRTV(rtv, clearColor);
	cl->setScreenRenderTarget(&rtv, 1, (window.depthFormat == DXGI_FORMAT_UNKNOWN) ? nullptr : &window.depthBuffer.dsvHandle.cpuHandle);

	if (win32_window::mainWindow == &window)
	{
		renderImGui(cl);
	}

	cl->transitionBarrier(backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	uint64 result = dxContext.executeCommandList(cl);

	window.swapBuffers();

	return result;
}

LONG NTAPI handleVectoredException(PEXCEPTION_POINTERS exceptionInfo)
{
	PEXCEPTION_RECORD exceptionRecord = exceptionInfo->ExceptionRecord;

	switch (exceptionRecord->ExceptionCode)
	{
	case DBG_PRINTEXCEPTION_WIDE_C:
	case DBG_PRINTEXCEPTION_C:

		if (exceptionRecord->NumberParameters >= 2)
		{
			ULONG len = (ULONG)exceptionRecord->ExceptionInformation[0];

			union
			{
				ULONG_PTR up;
				PCWSTR pwz;
				PCSTR psz;
			};

			up = exceptionRecord->ExceptionInformation[1];

			HANDLE hOut = GetStdHandle(STD_ERROR_HANDLE);

			if (exceptionRecord->ExceptionCode == DBG_PRINTEXCEPTION_C)
			{
				// Localized text will be incorrect displayed, if used not CP_OEMCP encoding.
				// WriteConsoleA(hOut, psz, len, &len, 0);

				// assume CP_ACP encoding
				if (ULONG n = MultiByteToWideChar(CP_ACP, 0, psz, len, 0, 0))
				{
					PWSTR wz = (PWSTR)alloca(n * sizeof(WCHAR));

					if (len = MultiByteToWideChar(CP_ACP, 0, psz, len, wz, n))
					{
						pwz = wz;
					}
				}
			}

			if (len)
			{
				WriteConsoleW(hOut, pwz, len - 1, &len, 0);
			}

		}
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	return EXCEPTION_CONTINUE_SEARCH;
}

int main(int argc, char** argv)
{
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	AddVectoredExceptionHandler(TRUE, handleVectoredException);

	dxContext.initialize();

	color_depth colorDepth = color_depth_8;

	dx_window window;
	window.initialize(TEXT("Main Window"), 1280, 800, colorDepth, DXGI_FORMAT_UNKNOWN, false);

	dx_window window2;
	window2.initialize(TEXT("Window 2"), 1280, 800, colorDepth, DXGI_FORMAT_UNKNOWN, false);

	setMainWindow(&window);

	D3D12_RT_FORMAT_ARRAY screenRTFormats = {};
	screenRTFormats.NumRenderTargets = 1;
	screenRTFormats.RTFormats[0] = (colorDepth == color_depth_8) ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_R10G10B10A2_UNORM;

	initializeImGui(screenRTFormats);

	LARGE_INTEGER perfFreqResult;
	QueryPerformanceFrequency(&perfFreqResult);
	perfFreq = (float)perfFreqResult.QuadPart;

	QueryPerformanceCounter(&lastTime);


	user_input input = {};
	float dt;


	uint64 fenceValues[NUM_BUFFERED_FRAMES] = {};

	fenceValues[NUM_BUFFERED_FRAMES - 1] = dxContext.renderQueue.signal();

	uint64 frameID = 0;

	while (newFrame(input, dt))
	{
		dxContext.renderQueue.waitForFence(fenceValues[window.currentBackbufferIndex]);

		dxContext.newFrame(frameID);

		ImGui::Begin("Test");
		ImGui::Text("Hallo");
		ImGui::End();

		float clearColor1[] = { 0.f, 0.f, 0.f, 1.f };
		float clearColor2[] = { 1.f, 1.f, 0.f, 1.f };

		fenceValues[window.currentBackbufferIndex] = renderToWindow(window, clearColor1);
		renderToWindow(window2, clearColor2);

		++frameID;
	}

	dxContext.quit();
}
