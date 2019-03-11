#include <stdlib.h>
unsigned int sceLibcHeapExtendedAlloc = 1;			/* Switch to dynamic allocation */
size_t       sceLibcHeapSize = 256 * 1024 * 1024;	/* Set up heap area upper limit as 256 MiB */

#include "../Plugins/PlayStation4/PS4Window.h"
#include "../Plugins/PlayStation4/Ps4SoundSystem.h"
#include "../Plugins/PlayStation4/PS4Input.h"

#include <iostream>

#include "ExampleRenderer.h"

using namespace NCL;
using namespace NCL::PS4;

int main(void) {
	PS4Window*w = (PS4Window*)Window::CreateGameWindow("PS4 Example Code", 1920, 1080);

	ExampleRenderer renderer(w);
	//
	PS4Input		input		= PS4Input();
	
	Ps4SoundSystem*	soundSystem = new Ps4SoundSystem(8);
	
	while (true) {
		input.Poll();

		renderer.Update(2);
		renderer.Render();
	
		if (input.GetButton(0)) {
			std::cout << "LOL BUTTON" << std::endl;
		}
	
		if (input.GetButton(1)) {
			return 1;
		}
	}
	
	return 1;
}

