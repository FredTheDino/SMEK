#include "util/util.h"

#include "game.h"
#include "test.h"
#include "renderer/opengl.h"
#include "renderer/renderer.h"
#include "asset/asset.h"

#include "math/smek_mat4.h"
#include "math/smek_math.h"

#include "imgui/imgui.h"

GameState *_global_gs;
#ifndef TESTS
GameState *GAMESTATE() { return _global_gs; }
#endif

bool should_update(GSUM gsmu) {
    return gsmu == GSUM::UPDATE || gsmu == GSUM::UPDATE_AND_RENDER;
}

bool should_draw(GSUM gsmu) {
    return gsmu == GSUM::RENDER || gsmu == GSUM::UPDATE_AND_RENDER;
}

f32 delta() { return GAMESTATE()->delta; }
f32 time() { return GAMESTATE()->time; }
u32 frame() { return GAMESTATE()->frame; }

GFX::RenderTexture target;

void init_game(GameState *gamestate, int width, int height) {
    _global_gs = gamestate;
    GAMESTATE()->main_thread = SDL_GetThreadID(NULL);

    Asset::load("assets.bin");

    GFX::init(GAMESTATE(), width, height);

    GFX::lighting()->sun_direction = Vec3(0.0, 1.0, 0.0);
    *GFX::debug_camera() = GFX::Camera::init();
    GFX::debug_camera()->position = Vec3(0.0, 0.2, 0.0);

    *GFX::gameplay_camera() = GFX::Camera::init();
    GFX::gameplay_camera()->position = Vec3(0.0, 0.2, 0.0);

    GAMESTATE()->running = true;

    Input::bind(Ac::MoveX, 0, SDLK_a, -1.0);
    Input::bind(Ac::MoveX, 1, SDLK_d, 1.0);
    Input::bind(Ac::MoveY, 0, SDLK_q, 1.0);
    Input::bind(Ac::MoveY, 1, SDLK_e, -1.0);
    Input::bind(Ac::MoveZ, 0, SDLK_w, -1.0);
    Input::bind(Ac::MoveZ, 1, SDLK_s, 1.0);
    Input::bind(Ac::Jump, 0, SDLK_SPACE, 1.0);
    Input::bind(Ac::Shoot, 0, SDLK_f, 1.0);
    Input::bind(Ac::MouseToggle, 0, SDLK_m);
    Input::bind(Ac::Rebind, 1, SDLK_r);

    Player player;
    GAMESTATE()->event_queue.push(entity_event(player));

    Light l = Light();
    GAMESTATE()->lights[0] = GAMESTATE()->entity_system.add(l);
    GAMESTATE()->lights[1] = GAMESTATE()->entity_system.add(l);
}

void reload_game(GameState *game) {
    _global_gs = game;
    GFX::reload(game);
    Asset::reload();
#ifndef IMGUI_DISABLE
    ImGui::SetCurrentContext((ImGuiContext *)game->imgui_context);
#endif
    target = GFX::RenderTexture::create(500, 500, true, true);
}

void update() {
    if (Input::released(Ac::MouseToggle)) {
        GAMESTATE()->input.mouse_capture ^= 1;
    }

    EventSystem::handle_events();

    // Debug camera movement, overwritten by player currently.
    if (GFX::current_camera() == GFX::debug_camera()) {
        Vec3 move = { Input::value(Ac::MoveX), 0, Input::value(Ac::MoveZ) };
        Vec2 turn = Input::mouse_move();
        move = move * delta();
        turn = turn * delta();
        GFX::debug_camera()->turn(turn.y, turn.x);
        GFX::debug_camera()->move_relative(move);
        GFX::debug_camera()->move(
            Vec3(0, Input::value(Ac::MoveY) * delta(), 0));
    }
    GAMESTATE()->entity_system.update();
}

void draw() {
    target.use();

    static f32 t = 0;
#ifndef IMGUI_DISABLE
    ImGui::SliderFloat("Time", &t, 0.0, 16.0, "%.2f");

    static bool use_debug_camera = GFX::current_camera() == GFX::debug_camera();
    ImGui::Checkbox("Debug camera", &use_debug_camera);
    GFX::set_camera_mode(use_debug_camera);

    GFX::Lighting *lighting = GFX::lighting();
    ImGui::InputFloat3("Sun Color", lighting->sun_color._, 3);
    ImGui::InputFloat3("Sun Direction", lighting->sun_direction._, 3);
    lighting->sun_direction = normalized(lighting->sun_direction);
    ImGui::InputFloat3("Ambient Color", lighting->ambient_color._, 3);

    if (ImGui::Button("Add Light")) {
        static int i = 0;
        Light l;
        l.position.x = i++;
        l.color = Vec3(1.0, 1.0, 0.0);
        GAMESTATE()->entity_system.add(l);
    }
#endif

    glClearColor(0.2, 0.1, 0.3, 1); // We don't need to do this...
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GFX::MasterShader shader = GFX::master_shader();
    shader.use();
    shader.upload_t(time());

    GFX::current_camera()->upload(shader);
    shader.upload_bones(0, nullptr);

    shader.upload_sun_dir(GFX::lighting()->sun_direction);
    shader.upload_sun_color(GFX::lighting()->sun_color);
    shader.upload_ambient_color(GFX::lighting()->ambient_color);

    Vec3 position = Vec3(3, 0.5, 3);
    {
        Light *l = GAMESTATE()->entity_system.fetch<Light>(GAMESTATE()->lights[0]);
        l->position = position + Vec3(0.5, 1.0 + sin(time()), 0.0);
        l->color = Vec3(sin(time()) * 0.5 + 0.5, cos(time()) * 0.5 + 0.5, 0.2);
    }

    {
        Light *l = GAMESTATE()->entity_system.fetch<Light>(GAMESTATE()->lights[1]);
        l->position = position + Vec3(1.0 + cos(time()), 1.0, sin(time()));
        l->color = Vec3(0.5, 0.5, 0.9);
    }

    shader.upload_lights(GFX::lighting()->light_positions,
                         GFX::lighting()->light_colors);

    const i32 grid_size = 10;
    const f32 width = 0.005;
    const Vec4 color = GFX::color(7) * 0.4;
    for (f32 x = 0; x <= grid_size; x += 0.5) {
        GFX::push_point(Vec3(x, 0, x), GFX::color(2), width * 10);
        GFX::push_line(Vec3(x, 0, grid_size), Vec3(x, 0, -grid_size), color, width);
        GFX::push_line(Vec3(-x, 0, grid_size), Vec3(-x, 0, -grid_size), color, width);
        GFX::push_line(Vec3(grid_size, 0, x), Vec3(-grid_size, 0, x), color, width);
        GFX::push_line(Vec3(grid_size, 0, -x), Vec3(-grid_size, 0, -x), color, width);
    }

    Asset::fetch_texture("RGBA")->bind(0);
    shader.upload_tex(0);

    GAMESTATE()->entity_system.draw();

#if 0
    GFX::Skin *mesh = Asset::fetch_skin("SKIN_SWINGING_CUBE");
    Mat model_matrix = Mat::translate(Math::cos(time()) * 0.2, Math::sin(time()) * 0.2, -0.5) * Mat::scale(1.0);
    shader.upload_model(model_matrix);
    shader.upload_t(time());
    mesh->draw();

    Vec3 p(Math::cos(time()) * 0.2, Math::sin(time()) * 0.2, -0.5);
    GFX::push_point(Vec3(p.x, p.y, p.z));
#endif

    AssetID skin, skel, anim;
    skin = "SKIN_UNTITLED";
    // TODO(ed): Not drawing...
    // Asset::fetch_skin(skin)->draw();
    skel = "SKEL_UNTITLED";
    // anim = "ANIM_SKINNEDMESHACTION_RIGGED_SIMPLE_CHARACTER";
    anim = "ANIM_ARMATUREACTION_001_UNTITLED";
    // GFX::AnimatedMesh::init(skin, skel, anim).draw_at(t * 60);

    //Asset::fetch_skeleton(skel)->draw();

    GFX::debug_shader().use();
    GFX::current_camera()->upload(GFX::debug_shader());
    GFX::draw_primitivs();

#ifndef IMGUI_DISABLE
    ImGui::Begin("Game View");
    {
        ImGui::Image((void *)target.color, ImVec2(target.width, target.height), ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();

    ImGui::Begin("Depth");
    {
        ImGui::Image((void *)target.depth_output, ImVec2(target.width, target.height), ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
    ImGui::Begin("Networking");
    {
        static int serverport = 8888;
        ImGui::SetNextItemWidth(150);
        ImGui::PushID(&serverport);
        ImGui::InputInt("port", &serverport);
        ImGui::PopID();
        if (ImGui::Button("Create server")) {
            GAMESTATE()->network.setup_server(serverport);
        }

        ImGui::Separator();

        static char ip[64] = "127.0.0.1";
        static int connectport = 8888;
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("server address", ip, IM_ARRAYSIZE(ip));
        ImGui::SetNextItemWidth(150);
        ImGui::PushID(&connectport);
        ImGui::InputInt("port", &connectport);
        ImGui::PopID();
        if (ImGui::Button("Connect to server")) {
            GAMESTATE()->network.connect_to_server(ip, connectport);
        }
        if (GAMESTATE()->network.server_handle.active) {
            //TODO(gu) Generate instead of writing manually. Custom imgui widget?
            static PackageType package_type = PackageType::B;
            static int a_a = 0;
            static int b_a = 0;
            static int b_b = 0;

            static int type_current_id = 0;
            ImGui::Combo("", &type_current_id, "A\0B\0\0");
            
            Package package;
            package.type = (PackageType) type_current_id;
            switch (package.type) {
            case PackageType::A:
                ImGui::InputInt("A::a", &a_a);
                package.PKG_A.a = a_a;
                break;
            case PackageType::B:
                ImGui::InputInt("B::a", &b_a);
                ImGui::InputInt("B::b", &b_b);
                package.PKG_B.a = b_a;
                package.PKG_B.b = b_b;
                break;
            default:
                break;
            }

            if (ImGui::Button("Send")) {
                GAMESTATE()->network.server_handle.send(package);
            }
        }
    }
    ImGui::End();
#endif

    GFX::resolve_to_screen(target);
}

GameState update_game(GameState *game, GSUM mode) { // Game entry point
    _global_gs = game;
    if (should_update(mode)) { update(); }
    if (should_draw(mode)) { draw(); }
    return *game;
}
