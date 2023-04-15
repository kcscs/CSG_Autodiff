#include <Dragonfly/editor.h>		 //inlcludes most features
#include <Dragonfly/detail/buffer.h> //will be replaced
#include <Dragonfly/detail/vao.h>	 //will be replaced
#include "app.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <wtypes.h>

	__declspec(dllexport) DWORD NvOptimusEnablement = 0;
	//__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

#ifdef __cplusplus
}
#endif

int main(int argc, char* args[])
{
	df::Sample sam("Renderer", 640, 480, (df::Sample::FLAGS::DEFAULT & ~df::Sample::FLAGS::INIT_RENDERDOC & ~df::Sample::FLAGS::V_SYNC) ); //| df::Sample::FLAGS::V_SYNC
	// df::Sample simplifies OpenGL, SDL, ImGui, RenderDoc in the render loop, and handles user input via callback member functions in priority queues
								// Implements a camera event class with handles

	sam.AddHandlerClass<df::ImGuiHandler>(10);	// static handle functions only
	{
		App app(sam, 480.0f/720);

		sam.Run([&](float deltaTime) //delta time in ms
			{
				app.DrawUI();
				app.Update();
				app.Render();
			}
		);
	}
	return 0;
}
