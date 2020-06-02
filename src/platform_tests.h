#define SOME_ACTION ((Ac) 0)

#define DEFAULT_INPUT_TEST \
    GameInput input;\
    const u32 button_1 = 2;\
    const u32 button_2 = 5;\
    const u32 button_3 = 232;\
    input.bind(SOME_ACTION, 0, button_1,  1.0);\
    input.bind(SOME_ACTION, 1, button_2, -1.0)

#include "test.h"
TEST_CASE("platform_input_press", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_3, true);
    return input.state[0] == 0.0;
});

TEST_CASE("platform_input_press", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, true);
    return input.state[0] == 1.0;
});

TEST_CASE("platform_input_press", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, true);
    input.update_press(button_1, false);
    input.update_press(button_3, true);
    input.update_press(button_1, true);
    return input.state[0] == 1.0;
});

TEST_CASE("platform_input_press", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_3, true);
    input.update_press(button_2, true);
    input.update_press(button_3, false);
    return input.state[0] == -1.0;
});

TEST_CASE("platform_input_press", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, false);
    input.update_press(button_1, false);
    input.update_press(button_1, false);
    return input.state[0] == 0.0;
});

TEST_CASE("platform_input_rebind", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, true);
    input.update_press(button_1, false);
    input.unbind(SOME_ACTION, 0);
    input.update_press(button_1, true);
    input.update_press(button_1, false);
    return input.state[0] == 0.0;
});

TEST_CASE("platform_input_rebind", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_3, true);
    input.update_press(button_3, false);
    input.unbind(SOME_ACTION, 0);
    input.bind(SOME_ACTION, 0, button_3);
    input.update_press(button_1, true);
    input.update_press(button_1, false);
    return input.state[0] == 0.0;
});

TEST_CASE("platform_input_rebind", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_3, true);
    input.update_press(button_3, false);
    input.unbind(SOME_ACTION, 0);
    input.bind(SOME_ACTION, 0, button_3);
    input.update_press(button_3, true);
    return input.state[0] == 1.0;
});

TEST_CASE("platform_input_rebind", {
    DEFAULT_INPUT_TEST;
    input.update_press(button_1, true);
    input.unbind(SOME_ACTION, 0);
    input.update_press(button_1, false);
    input.bind(SOME_ACTION, 0, button_1);
    input.update_press(button_1, false);
    return input.state[0] == 0.0;
});

