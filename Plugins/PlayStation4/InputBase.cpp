#include "InputBase.h"

using namespace NCL::PS4;

InputBase::InputBase()
{
	ResetInput();
}


InputBase::~InputBase()
{
}

void InputBase::ResetInput() {
	for (int i = 0; i < MAX_AXIS; ++i) {
		axis[i].x = 0.0f;
		axis[i].y = 0.0f;
	}
	for (int i = 0; i < MAX_BUTTONS; ++i) {
		buttons[i] = 0.0f;
	}
}

// Axis 0 Left Joystick
// Axis 1 Right Joystick
// Axis 2 Directional buttons
// + Right/Up   - Left/Down
// x L/R        y Up/Down
// [-1,1]
AXIS	InputBase::GetAxis(unsigned int i) {
	if (i > MAX_AXIS) {
		return AXIS();
	}
	return axis[i];
}

//Button 0 Triangle Button 1 Circle Button 2 Cross Button 3 Square
//Button 4 L2       Button 5 R2     Button 6 L1    Button 7 L2
//Button 8 L3       Button 9 R3
float	InputBase::GetButton(unsigned int i) {
	if (i > MAX_BUTTONS) {
		return 0.0f;
	}
	return buttons[i];
}

bool	InputBase::GetButtonDown(unsigned int i) {
	if (i > MAX_BUTTONS) {
		return false;
	}
	return buttons[i] > 0.5f ? true : false;
}