#pragma once
#include "math/types.h"

namespace Input {

///*
// The list of possible buttons that can be pressed.
enum class Action {
    AButton,
    BButton,

    Rebind,

    NUM_ACTIONS
};

typedef void(*RebindFunc)(Action, u32, f32);
typedef void(*BindFunc)(Action, u32, u32, f32);

///*
// Holds the content of this frames input.
struct Input {
    f32 current_frame[(u32) Action::NUM_ACTIONS];
    f32 last_frame[(u32) Action::NUM_ACTIONS];

    RebindFunc rebind_func;
    BindFunc bind_func;
};

///*
// Rebinds the given Action to the next key pressed down.
void rebind(Action name, u32 slot=0, f32 value=1.0);

///*
// Binds the given Action to be set to "value" when pressed.
void bind(Action name, u32 slot, u32 button, f32 value=1.0);

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

///*
// Returns the floating point value for this input.
f32 value(Action name);

}

///*
// Imports this into global namespace to
// make it easier to use the input functions.
using Ac = Input::Action;
