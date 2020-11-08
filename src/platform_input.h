//
// Handles input on the platform layer, this is then passed to the
// game in the GameState struct.
//
// Most of the stuff here should not really be messed with, since it
// is probabbly called from the game layer.
//

// All the state the platform layer needs to know if keys were pressed
// or released.
struct GameInput {
    using Button = i32;

    struct ButtonPress {
        Ac action;
        f32 value;
    };

    // Button state.
    Button action_to_input[(u32)Ac::NUM_ACTIONS][2] = {};
    std::unordered_map<Button, ButtonPress> input_to_action;
    f32 state[(u32)Ac::NUM_ACTIONS] = {};

    // Mouse state
    Vec2 mouse_pos;
    Vec2 mouse_move;
    bool capture_mouse;

    // Rebinding state
    bool rebinding;
    Ac rebind_action;
    u32 rebind_slot;
    f32 rebind_value;

    // Callback state

    using Callback = void (*)();
    std::unordered_map<Button, Callback> callback_table;

    void bind(Ac action, u32 slot, Button button, f32 value = 1.0) {
        ASSERT(slot < LEN(action_to_input[0]), "Invalid binding slot, max %d. (%d)",
               LEN(action_to_input[0]), slot);
        if (input_to_action.count(button))
            WARN("Button cannot be bound to multiple actions (%d)", button);
        action_to_input[(u32)action][slot] = button;
        input_to_action[button] = { action, value };
    }

    bool unbind(Ac action, u32 slot) {
        ASSERT(slot < LEN(action_to_input[0]), "Invalid binding slot, max %d. (%d)",
               LEN(action_to_input[0]), slot);

        Button button = action_to_input[(u32)action][slot];
        action_to_input[(u32)action][slot] = 0;

        if (button == 0) { return false; }
        input_to_action.erase(button);
        return true;
    }

    bool eaten_by_rebind(Button button) {
        if (!rebinding) return false;
        rebinding = false;
        unbind(rebind_action, rebind_slot);
        bind(rebind_action, rebind_slot, button, rebind_value);
        return true;
    }

    void update_press(Button button, bool down) {
        if (input_to_action.count(button)) {
            ButtonPress press = input_to_action[button];
            if (down) {
                state[(u32)press.action] = down * press.value;
            } else {
                if (state[(u32)press.action] == press.value) {
                    state[(u32)press.action] = 0.0;
                }
            }
        }
    }

    Button as_modded(Button button, i32 mod) {
        return button | (mod << 5);
    }

    void clear_callbacks() {
        callback_table.clear();
    }

    void add_callback(Button button, i32 mod, Callback callback) {
        callback_table[as_modded(button, mod)] = callback;
    }

    bool trigger_callbacks(Button button, i32 mod) {
        if (callback_table.contains(as_modded(button, mod))) {
            callback_table[as_modded(button, mod)]();
            return true;
        }
        return false;
    }
} global_input = {};

///# Input wrappers
// Wrappers for the platform input layer, alternatives are supplied for
// the game layer.
//
void platform_rebind(Ac action, u32 slot, f32 value);
void platform_bind(Ac action, u32 slot, u32 button, f32 value);

// See documentation in input.h
void platform_rebind(Ac action, u32 slot, f32 value) {
    ASSERT(slot < LEN(global_input.action_to_input[0]), "Invalid binding slot, max %d. (%d)",
           LEN(global_input.action_to_input[0]), slot);
    global_input.rebinding = true;
    global_input.rebind_action = action;
    global_input.rebind_slot = slot;
    global_input.rebind_value = value;
}

void platform_bind(Ac action, u32 slot, u32 button, f32 value) {
    global_input.unbind(action, slot);
    global_input.bind(action, slot, button, value);
}

void platform_callback(u32 button, u32 mods, void (*callback)()) {
    global_input.add_callback(button, mods, callback);
}
