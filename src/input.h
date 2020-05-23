#pragma once
#include "math/types.h"

namespace Input {

///*
// The list of possible buttons that can be pressed.
enum class Action {
    AButton,
    BButton,

    NUM_INPUTS
};

///*
// Holds the content of this frames input.
struct Input {
    f32 current_frame[(u32) Action::NUM_INPUTS];
    f32 last_frame[(u32) Action::NUM_INPUTS];
};

///*
// Returns true if the input is being held down this frame.
bool down(Action name);

///*
// Returns true if the input is not being held down this frame.
bool up(Action name);

///*
// Returns true if the input was released this frame.
bool released(Action name);

///*
// Returns true if the input was pressed this frame.
bool pressed(Action name);

}
