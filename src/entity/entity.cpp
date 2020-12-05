#include "entity.h"
#include "../asset/asset.h"
#include "../renderer/renderer.h"
#include "../network/network.h"
#include "../game.h"
#include "../test.h"
#include "imgui/imgui.h"

// These helper functions make it easier to
// create the ClassName::imgui() functions, where
// it's needed. The code will be stubbed if imgui
// and make sure each id is unique to avoid duplicate
// editing.
#ifdef IMGUI_ENABLE

#define IMPL_IMGUI(ClassName, f)     \
    void ClassName ::imgui() {       \
        ImGui::PushID(this);         \
        ImGui::Text(STR(ClassName)); \
        auto f_inst = f;             \
        f_inst();                    \
        ImGui::PopID();              \
    }

#else

#define IMPL_IMGUI(ClassName, f) \
    void ClassName ::imgui() {}

#endif

EntitySystem *entity_system() {
    return &GAMESTATE()->entity_system;
}

void Light::update() {
    if (length_squared(color) != 0.0) {
        if (light_id == NONE) {
            // Try to aquire a light ID
            for (u32 i = 0; i < GFX::MAX_LIGHTS; i++) {
                if (length_squared(GFX::lighting()->light_colors[i]) != 0)
                    continue;
                light_id = i;
                break;
            }
        }
        if (light_id == NONE) return;
        GFX::lighting()->light_colors[light_id] = color;
        GFX::lighting()->light_positions[light_id] = position;
    } else {
        if (light_id != NONE) on_remove();
    }
}

IMPL_IMGUI(Light, ([&] {
               i32 copy_id = light_id;
               ImGui::InputInt("#Light ID:", &copy_id);
               ImGui::InputFloat3("Position", position._);
               ImGui::ColorEdit3("Color", color._);
               ImGui::Checkbox("Draw As Point", &draw_as_point);
           }))

void Block::on_create() {
    GAMESTATE()->physics_engine.add_box({ entity_id, Vec3(), Vec3(), scale });
}

void Block::draw() {
    GFX::push_mesh("CUBE", "TILES", position, rotation, scale);
}

void Block::update() {
    for (auto [_, e] : GAMESTATE()->entity_system.entities) {
        if (e->type != EntityType::PLAYER) continue;

    }
}

void Light::draw() {
    if (draw_as_point) {
        if (light_id == NONE) {
            GFX::push_point(position + Vec3(0.01, 0.0, 0.0), Color4(1.0, 0.0, 0.0, 1.0), 0.07);
            GFX::push_point(position, Color4(color.r, color.g, color.b, 0.2), 0.05);
        } else {
            GFX::push_point(position, Color4(color.r, color.g, color.b, 1.0), 0.1);
        }
    }
}

void Light::on_remove() {
    // Black means the color isn't used.
    if (light_id != NONE) {
        GFX::lighting()->light_colors[light_id] = Color3();
        light_id = NONE;
    }
}

void LightUpdate::callback() {
    if (!GAMESTATE()->entity_system.is_valid(entity_id)) {
        WARN("Received event position update for invalid entity id {}", entity_id);
        return;
    }
    Light *l = GAMESTATE()->entity_system.fetch<Light>(entity_id);
    l->position = Vec3::from(position);
    l->color = Color3::from(color);
    l->draw_as_point = draw_as_point;
}

void PlayerInput::callback() {
    // This could be per client instead. In that case we would iterate all
    // players and compare their entity id client id prefix with the network
    // client id prefix.
    if (!GAMESTATE()->entity_system.is_valid(entity_id)) {
        WARN("Received player input for invalid entity id {}", entity_id);
        return;
    }
    Player *p = GAMESTATE()->entity_system.fetch<Player>(entity_id);
    p->last_input = *this;
}

void Player::on_create() {
    GAMESTATE()->physics_engine.add_box({ entity_id, Vec3(), Vec3(), scale, 1 });
}

void Player::update() {
    bool own = GAMESTATE()->entity_system.have_ownership(entity_id);
    bool debug_camera = GFX::current_camera() == GFX::debug_camera();
    if (GAMESTATE()->network.server_listening) {
        if (own && !debug_camera) {
            update_input();
        }
        update_position();
        if (own && !debug_camera) {
            update_camera();
        }
    } else if (GAMESTATE()->network.server_handle.active) {
        if (own) {
            if (!debug_camera) {
                update_input();
                update_camera();
            }
        }
    } else if (!debug_camera) {
        update_input();
        update_position();
        update_camera();
    }
}

void Player::update_input() {
    Package pkg;
    pkg.header.type = PackageType::EVENT;
    pkg.EVENT.event.type = EventType::PLAYER_INPUT;
    PlayerInput player_input;
    player_input.entity_id = this->entity_id;
    Vec2 turn = Input::mouse_move()
                * GAMESTATE()->player_mouse_sensitivity
                * delta();
    rotation = normalized(H::from(0.0, -turn.x, 0.0)
                          * rotation
                          * H::from(-turn.y, 0.0, 0.0));
    rotation.to(player_input.rotation);
    Vec3 v_move = {
        Input::value(Ac::MoveX),
        Input::value(Ac::MoveY),
        Input::value(Ac::MoveZ),
    };
    v_move = v_move * GAMESTATE()->player_movement_speed; //TODO *=
    v_move.to(player_input.move_axis);
    player_input.jump = Input::pressed(Ac::Jump);
    player_input.shot = Input::pressed(Ac::Shoot);
    pkg.EVENT.event.PLAYER_INPUT = player_input;

    if (GAMESTATE()->network.server_handle.active) {
        GAMESTATE()->network.server_handle.send(&pkg);
    } else {
        last_input = player_input;
    }
}

void Player::update_position() {
    const f32 VELOCITY_EPSILON = 0.001;

    if (!GAMESTATE()->entity_system.have_ownership(entity_id)) {
        Quat input_rotation = Quat(last_input.rotation);
        if (length_squared(input_rotation) != 0.0) {
            rotation = input_rotation;
        }
    }
    Vec3 move(last_input.move_axis);
    if (length_squared(move) > 1.0) {
        move = move / length(move); //TODO /=
    }
    f32 drag_coef = Math::pow(0.05, delta());
    velocity.x = velocity.x * drag_coef;
    velocity.y += -4.82 * delta(); // temp gravity
    velocity.z = velocity.z * drag_coef;
    velocity += rotation
                * Vec3(move.x, 0.0, move.z)
                * GAMESTATE()->player_movement_speed
                * delta();
    // Plane collision
    if (position.y <= FLOOR) {
        position.y = FLOOR;
        velocity.y = 0.0;

        // If grounded
        // TODO(gu) check if colliding with floor instead
        if (last_input.jump && velocity.y < VELOCITY_EPSILON) {
            velocity.y = GAMESTATE()->player_jump_speed;
        }
    }
    if (last_input.shot) {
        INFO("Pew!");
    }
    position += velocity * delta();
}

void Player::update_camera() {
    GFX::gameplay_camera()->position = position + Vec3(0.0, 0.9, 0.0);
    GFX::gameplay_camera()->rotation = rotation;
}

IMPL_IMGUI(Player, ([&]() {
               ImGui::SliderFloat("Jump speed",
                                  &GAMESTATE()->player_jump_speed,
                                  0.0, 10.0,
                                  "%.2f");
               ImGui::SliderFloat("Movement speed",
                                  &GAMESTATE()->player_movement_speed,
                                  0.0, 10.0,
                                  "%.2f");
               ImGui::SliderFloat("Mouse sensitivity",
                                  &GAMESTATE()->player_mouse_sensitivity,
                                  0.0, 2.0,
                                  "%.2f");
           }))

void Player::draw() {
    scale = Vec3(1., 2., 3.) * 0.3;
    GFX::push_mesh("MONKEY", "TILES", position, rotation, scale);
}

void PlayerUpdate::callback() {
    if (!GAMESTATE()->entity_system.is_valid(entity_id)) {
        WARN("Received player update for invalid entity id {}", entity_id);
        return;
    }
    Player *p = GAMESTATE()->entity_system.fetch<Player>(entity_id);
    p->position = Vec3(position);
    if (!GAMESTATE()->entity_system.have_ownership(entity_id)) {
        p->rotation = H(rotation);
    }
}

void SoundEntity::update() {
    remove |= !GAMESTATE()->audio_struct->is_valid(audio_id);
}

void SoundEntity::imgui() {
#ifdef IMGUI_ENABLE
    Audio::SoundSource *source = GAMESTATE()->audio_struct->fetch_source(audio_id);
    if (!source) return;
    ImGui::BeginChild("Sound entities");
    ImGui::PushID(this);
    ImGui::Indent();
    ImGui::Text("SoundEntity");
    ImGui::SameLine();
    ImGui::Text("%.2f/%.2f",
                (f32)source->sample / source->sample_rate,
                (f32)source->num_samples / source->channels / source->sample_rate);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0);
    f32 gain = source->gain;
    if (ImGui::SliderFloat("Gain", &gain, 0.0, 1.0, "%.2f")) {
        GAMESTATE()->audio_struct->lock();
        source->gain = gain;
        GAMESTATE()->audio_struct->unlock();
    }
    ImGui::SameLine();
    if (ImGui::Button("Play/pause")) GAMESTATE()->audio_struct->toggle_pause_sound(audio_id);
    ImGui::SameLine();
    remove |= ImGui::Button("Stop");
    ImGui::Unindent();
    ImGui::PopID();
    ImGui::EndChild();
#endif
}

void SoundEntity::draw() {}

void SoundEntity::on_create() {
    audio_id = GAMESTATE()->audio_struct->play_sound(*this);
}

void SoundEntity::on_remove() {
    GAMESTATE()->audio_struct->stop_sound(audio_id);
}

bool EntitySystem::is_valid(EntityID id) {
    return entities.contains(id);
}

bool EntitySystem::have_ownership(EntityID id) {
    int lock_success = SDL_LockMutex(m_client_id);
    ASSERT(lock_success == 0, "Unable to lock mutex: {}", SDL_GetError());
    defer { SDL_UnlockMutex(m_client_id); };
    return client_id == (id & 0xFF00000000000000);
}

void EntitySystem::remove(EntityID id) {
    ASSERT(is_valid(id), "Removing invalid entity id {}", id);
    entities[id]->on_remove();
    delete entities[id];
    entities.erase(id);
}

void EntitySystem::remove_all() {
    for (auto [_, e] : entities) {
        e->on_remove();
        delete e;
    }
    entities.clear();
}

void EntitySystem::update() {
#if IMGUI_ENABLE
    for (auto [id, e] : entities) {
        Physics::AABody box = {};
        if (Vec3 *p = get_field_by_name_no_warn<Vec3>(e, FieldName::position)) {
            box.position = *p;
        }

        // Size for boxes that doesn't have a scale.
        box.half_size = Vec3(1.0, 1.0, 1.0) * 0.2;
        if (Vec3 *s = get_field_by_name_no_warn<Vec3>(e, FieldName::scale)) {
            box.half_size = (*s) * 0.5;
        }

        Vec3 start = GFX::current_camera()->position;
        Vec3 dir = GFX::current_camera()->get_forward();
        Physics::Manifold manifold = Physics::collision_line_aabody(start, dir, &box);
        // TODO(ed): Only select the closest.
        bool hovering = manifold.t > 0;

        Color4 nothing_color = { 0., 0., 0., 1. };
        Color4 hovering_color = { 0., 0.5, 0.5, 1. };
        Color4 selected_color = { 0., 0.5, 0., 1. };
        Color4 selected_and_hover_color = { 0., 0.5, 0.8, 1. };

        if (hovering && Input::pressed(Ac::ESelect)) {
            if (selected.contains(id))
                selected.erase(id);
            else
                selected.insert(id);
        }

        if (hovering) {
            if (selected.contains(id)) {
                Physics::draw_aabody(box, selected_and_hover_color);
            } else {
                Physics::draw_aabody(box, hovering_color);
            }
        } else if (selected.contains(id)) {
            Physics::draw_aabody(box, selected_color);
        } else {
            Physics::draw_aabody(box, nothing_color);
        }
    }
#endif

    for (auto [_, e] : entities) {
        e->update();
    }
    for (auto i = entities.begin(), last = entities.end(); i != last;) {
        BaseEntity *e = i->second;
        if (e->remove) {
            e->on_remove();
            delete e;
            i = entities.erase(i);
        } else {
            i++;
        }
    }
}

void EntitySystem::send_state(ClientHandle *handle) {
    Package pkg;
    pkg.header.type = PackageType::EVENT;
    pkg.EVENT.event.type = EventType::LIGHT_UPDATE;
    LightUpdate event;
    if (is_valid(GAMESTATE()->lights[0])) {
        Light *light = fetch<Light>(GAMESTATE()->lights[0]);
        event.entity_id = light->entity_id;
        light->position.to(event.position);
        light->color.to(event.color);
        event.draw_as_point = light->draw_as_point;
        pkg.EVENT.event.LIGHT_UPDATE = event;
        handle->send(&pkg);
    }
    if (is_valid(GAMESTATE()->lights[1])) {
        Light *light = fetch<Light>(GAMESTATE()->lights[1]);
        event.entity_id = light->entity_id;
        light->position.to(event.position);
        light->color.to(event.color);
        event.draw_as_point = light->draw_as_point;
        pkg.EVENT.event.LIGHT_UPDATE = event;
        handle->send(&pkg);
    }

    for (const auto &[entity_id, entity] : entities) {
        if (entity->type == EntityType::PLAYER) {
            Player *player = static_cast<Player *>(entity);
            Package pkg;
            pkg.header.type = PackageType::EVENT;
            pkg.EVENT.event.type = EventType::PLAYER_UPDATE;
            PlayerUpdate event;
            event.entity_id = entity_id;
            player->position.to(event.position);
            player->rotation.to(event.rotation);
            pkg.EVENT.event.PLAYER_UPDATE = event;
            handle->send(&pkg);
        }
    }
}

void EntitySystem::send_initial_state(ClientHandle *handle) {
    TRACE("Sending initial state to client");
    Package entity_package;
    entity_package.header.type = PackageType::EVENT;
    for (const auto &[_, entity] : entities) {
        bool send = true;
        switch (entity->type) {
        case EntityType::LIGHT:
            entity_package.EVENT.event = entity_event(static_cast<Light *>(entity));
            break;
        case EntityType::PLAYER:
            entity_package.EVENT.event = entity_event(static_cast<Player *>(entity));
            break;
        default:
            send = false;
            break;
        }
        if (send) {
            handle->send(&entity_package);
        }
    }
}

void EntitySystem::drop_client(u64 client_id) {
    for (const auto &[id, entity] : entities) {
        if ((id & CLIENT_MASK) == client_id) {
            TRACE("Dropping entity with id {}", id);
            entity->remove = true;
        }
    }
}

void EntitySystem::draw() {
    draw_imgui();
    for (auto [_, e] : entities) {
        e->draw();
    }
}

bool has_field_by_name(BaseEntity *e, FieldNameType name) {
    FieldList types = get_fields_for(e->type);
    for (i32 i = 0; i < types.num_fields; i++) {
        if (types.list[i].name == name)
            return true;
    }
    return false;
}

void *_fetch_field_by_name_helper(BaseEntity *e, FieldNameType name, const std::type_info &info) {
    FieldList types = get_fields_for(e->type);
    for (i32 i = 0; i < types.num_fields; i++) {
        Field field = types.list[i];
        if (field.name == name && field.typeinfo == info) {
            return ((u8 *)e) + field.offset;
        }
    }
    return nullptr;
}

const char *type_name(BaseEntity *e) {
    return entity_type_names[(u32)e->type];
}

EntityID EntitySystem::next_id() {
    int lock_success = SDL_LockMutex(m_client_id);
    ASSERT(lock_success == 0, "Unable to lock mutex: {}", SDL_GetError());
    defer { SDL_UnlockMutex(m_client_id); };
    u64 id = id_counter++;
    id &= ID_MASK;
    id |= client_id;
    return id;
}

TEST_CASE("entity_adding", {
    GAMESTATE()->logger.levels &= ~LogLevel::TRACE;
    int calls;
    struct TestEnt : public Entity {
        int *value;
        void update() { value[0]++; };
        void draw() {}
    };

    TestEnt a;
    a.value = &calls;
    auto a_id = entity_system()->add(a);

    TestEnt b;
    b.value = &calls;
    auto b_id = entity_system()->add(b);
    calls = 0;
    entity_system()->update();
    ASSERT(calls == 2, "Got {} calls to update.", calls);

    entity_system()->remove(b_id);
    calls = 0;
    entity_system()->update();
    ASSERT(calls == 1, "Got {} calls to update.", calls);
    return true;
});

TEST_CASE("entity_remove", {
    GAMESTATE()->logger.levels &= ~LogLevel::TRACE;
    int calls;
    struct TestEnt : public Entity {
        int *value;
        ~TestEnt() { value[0] *= 4; }
        void update() { value[0]++; };
        void draw() {}
    };

    TestEnt b;
    b.value = &calls;
    auto b_id = entity_system()->add(b);
    calls = 0;
    entity_system()->update(); // + 1
    ASSERT(calls == 1, "Got {} calls to update.", calls);

    entity_system()->remove(b_id); // * 4
    entity_system()->update();     // Should not add
    ASSERT(calls == 4, "Got {} calls to update.", calls);
    return true;
});

TEST_CASE("entity_id_valid", {
    GAMESTATE()->logger.levels &= ~LogLevel::TRACE;
    struct TestEnt : public Entity {
        void update() {};
        void draw() {}
    };

    TestEnt b;
    auto b_id = entity_system()->add(b);
    ASSERT(entity_system()->is_valid(b_id), "Id should be valid");
    entity_system()->remove(b_id); // * 4
    ASSERT(!entity_system()->is_valid(b_id), "Id should not be valid");
    return true;
});

TEST_CASE("entity fetch", {
    GAMESTATE()->logger.levels &= ~LogLevel::TRACE;
    struct TestEnt : public BaseEntity {
        int value = 1;
    };
    TestEnt a;
    TestEnt *a_ptr;

    ASSERT_EQ(a.value, 1);
    auto id = entity_system()->add(a);
    a_ptr = entity_system()->fetch<TestEnt>(id);
    ASSERT_EQ(a_ptr->value, 1);
    a_ptr->value = 2;
    ASSERT_EQ(a_ptr->value, 2);
    return true;
});

TEST_CASE("entity on_create", {
    GAMESTATE()->logger.levels &= ~LogLevel::TRACE;
    struct TestEnt : public BaseEntity {
        int value = 1;
        void on_create() override {
            value = 2;
        }
    };
    TestEnt a;
    ASSERT_EQ(a.value, 1);
    auto id = entity_system()->add(a);
    TestEnt *a_ptr = entity_system()->fetch<TestEnt>(id);
    ASSERT_EQ(a_ptr->value, 2);
    return true;
});

TEST_CASE("entity on_remove", {
    GAMESTATE()->logger.levels &= ~LogLevel::TRACE;
    struct TestEnt : public BaseEntity {
        int value = 1;
        void on_remove() override { value = 2; }
    };
    TestEnt a;
    ASSERT_EQ(a.value, 1);
    auto id = entity_system()->add(a);
    TestEnt *a_ptr = entity_system()->fetch<TestEnt>(id);
    ASSERT_EQ(a_ptr->value, 1);
    entity_system()->remove(id);
    ASSERT_EQ(a_ptr->value, 2);
    return true;
});

TEST_CASE("entity add_emplace", {
    u8 buffer[MAX_ENTITY_SIZE];
    emplace_entity((void *)buffer, EntityType::PLAYER);
    Player *a_ptr = (Player *)buffer;
    ASSERT_EQ(a_ptr->type, EntityType::PLAYER);
    ASSERT_EQ(a_ptr->position.x, 0);
    ASSERT_EQ(a_ptr->position.y, 0);
    ASSERT_EQ(a_ptr->position.z, 0);
    return true;
});

TEST_CASE("entity add_unknown_type", {
    GAMESTATE()->logger.levels &= ~LogLevel::TRACE;
    Player a;
    a.type = EntityType::PLAYER;
    a.position = Vec3(1, 2, 3);
    auto id = entity_system()->add_unknown_type(&a);
    Player *a_ptr = entity_system()->fetch<Player>(id);
    ASSERT_EQ(a_ptr->position.x, a.position.x);
    ASSERT_EQ(a_ptr->position.y, a.position.y);
    ASSERT_EQ(a_ptr->position.z, a.position.z);
    return true;
});

#undef IMPL_IMGUI
