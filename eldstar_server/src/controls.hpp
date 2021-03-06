#pragma once

#include "gl/camera.hpp"
#include "input.hpp"

namespace eldstar {
namespace controls {

void orbiting_camera_control(eldstar::window& w, gl::perspective_camera& camera) {
    glm::vec3 camera_delta = camera.position - camera.target;
    float scale = glm::length(camera_delta);
    glm::vec3 lateral_delta = glm::vec3(camera.position.x - camera.target.x, 0.0f, camera.position.z - camera.target.z);
    float lateral_scale = glm::length(lateral_delta);
    if (lateral_scale == 0.0f) lateral_scale = 1.0f;
    lateral_delta *= (scale / lateral_scale);

    glm::vec3 right(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lateral_delta, 1.0f));

    float multiplier = 0.1f * static_cast<float>(w.delta_time);

    if (w.input_state->keyboard[GLFW_KEY_LEFT_SHIFT].action < 2)
        multiplier *= 3.0f;
    if (w.input_state->keyboard[GLFW_KEY_LEFT_CONTROL].action < 2)
        multiplier /= 3.0f;

    // Keyboard lateral control
    if (w.input_state->keyboard[GLFW_KEY_RIGHT].action < 2) {
        camera.position += right * multiplier;
        camera.target += right * multiplier;
    }
    if (w.input_state->keyboard[GLFW_KEY_LEFT].action < 2) {
        camera.position -= right * multiplier;
        camera.target -= right * multiplier;
    }
    if (w.input_state->keyboard[GLFW_KEY_DOWN].action < 2) {
        camera.position += lateral_delta * multiplier;
        camera.target += lateral_delta * multiplier;
    }
    if (w.input_state->keyboard[GLFW_KEY_UP].action < 2) {
        camera.position -= lateral_delta * multiplier;
        camera.target -= lateral_delta * multiplier;
    }

    // Keyboard altitude control
    if (w.input_state->keyboard[GLFW_KEY_PAGE_UP].action < 2) {
        camera.position += glm::vec3(0.0f, scale, 0.0f) * multiplier;
        camera.target += glm::vec3(0.0f, scale, 0.0f) * multiplier;
    }
    if (w.input_state->keyboard[GLFW_KEY_PAGE_DOWN].action < 2) {
        camera.position -= glm::vec3(0.0f, scale, 0.0f) * multiplier;
        camera.target -= glm::vec3(0.0f, scale, 0.0f) * multiplier;
    }

    // Scroll wheel zooming
    camera.position = camera.target + (camera_delta * powf(0.96f, w.input_state->scroll_delta.y));

    // Middle click panning
    if (w.input_state->mouse[2].action < 2) {
        glm::vec3 up = glm::cross(camera_delta, right);
        up *= (scale / glm::length(up));
        float pixels_to_world = glm::tan(glm::radians(camera.fov / 2.0f)) / static_cast<float>(w.gl_window.get_height());

        glm::vec3 move = (up * w.input_state->cursor_delta.y) - (right * w.input_state->cursor_delta.x);

        camera.position += move * pixels_to_world;
        camera.target += move * pixels_to_world;
    }

    // Orbiting
    float pixels_to_radians = -glm::radians(120.0f) / static_cast<float>(w.gl_window.get_height());
	float keyboard_speed = (10.0f * multiplier) / pixels_to_radians;
    glm::vec2 cursor = glm::vec2(0.0f, 0.0f);

    // Right click orbiting
    if (w.input_state->mouse[1].action < 2) cursor += w.input_state->cursor_delta;
    // Keyboard orbit support
    if (w.input_state->keyboard[GLFW_KEY_I].action < 2) cursor.y -= keyboard_speed;
    if (w.input_state->keyboard[GLFW_KEY_K].action < 2) cursor.y += keyboard_speed;
    if (w.input_state->keyboard[GLFW_KEY_J].action < 2) cursor.x -= keyboard_speed;
    if (w.input_state->keyboard[GLFW_KEY_L].action < 2) cursor.x += keyboard_speed;

    glm::vec4 orbit_delta = glm::vec4(camera.position - camera.target, 1.0f);

    float pitch = glm::atan(orbit_delta.y, glm::sqrt(glm::pow(orbit_delta.x, 2.0f) + glm::pow(orbit_delta.z, 2.0f)));
    float pitch_delta = cursor.y * pixels_to_radians;

    camera.position = camera.target + glm::vec3(glm::rotate(
        glm::abs(pitch - pitch_delta) > (glm::pi<float>() / 2.0f) ? glm::mat4(1.0f) : glm::rotate(
            glm::mat4(1.0f),
            cursor.y * pixels_to_radians,
            right
        ),
        cursor.x * pixels_to_radians,
        glm::vec3(0.0f, 1.0f, 0.0f)
    ) * orbit_delta);


    // Reset with Numpad 0
    if (w.input_state->keyboard[GLFW_KEY_KP_0].action == input::down) {
        camera.position = glm::vec3(0.0f, 650.0f, 1125.0f);
        camera.target = glm::vec3(0.0f);
        w.status("Free camera position has been reset.");
    }
}

void display_control(eldstar::window& w) {
    if (w.input_state->keyboard[GLFW_KEY_F].action == input::down) {
        w.wireframe = !w.wireframe;

        w.status(w.wireframe ? "Render mode set to: Wireframe" : "Render mode set to: Solid");
    }

    if (w.input_state->keyboard[GLFW_KEY_O].action == input::down) {
        w.color_mode = (w.color_mode + 1) % 3;

        std::string modes[3] = {
            "Zones",
            "Surface type",
            "Surface type [one-way normals]"
        };

        w.status("Color mode set to: " + modes[w.color_mode]);
    }

    if (w.input_state->keyboard[GLFW_KEY_TAB].action == input::down) {
        w.mirror_game_camera = !w.mirror_game_camera;
        w.status(w.mirror_game_camera ? "Mirroring game camera" : "Mirroring disabled, returning to free camera.");
    }

    if (w.input_state->keyboard[GLFW_KEY_C].action == input::down) {
        w.recording = !w.recording;
        w.status(w.recording ? "Dumping frames now: on" : "Dumping frames now: off");
    }
}

bool menu_control(eldstar::window& w, resource_manager& r, gl::perspective_camera& c, GLint color_id) {
    if (!w.active_menu) {
        if (w.input_state->keyboard[GLFW_KEY_M].action == input::down) {
            w.active_menu.reset(new menu(
                std::string("Eldstar Menu"),
                {
                    menu_option("Window resolution", [&w](void* ptr){
                        menu* m = reinterpret_cast<menu*>(ptr);
                        m->set_submenu(
                            "Resolution",
                            {
                                menu_option("592x400 [1x]", [&w](void* ptr) { w.gl_window.set_size(592, 400); }),
                                menu_option("854x480", [&w](void* ptr) { w.gl_window.set_size(854, 480); }),
                                menu_option("1280x720", [&w](void* ptr) { w.gl_window.set_size(1280, 720); }),
                                menu_option("1184x800 [2x]", [&w](void* ptr) { w.gl_window.set_size(1184, 800); }),
                                menu_option("1600x900", [&w](void* ptr) { w.gl_window.set_size(1600, 900); }),
                                menu_option("1920x1080", [&w](void* ptr) { w.gl_window.set_size(1920, 1080); }),
                            }
                        );
                    }),
                    menu_option("Free camera FOV", [&c](void* ptr){
                        menu* m = reinterpret_cast<menu*>(ptr);
                        m->set_submenu(
                            "Field of view",
                            {
                                menu_option("25 [ingame]", [&c](void* ptr) { c.fov = 25.0f; }),
                                menu_option("30", [&c](void* ptr) { c.fov = 30.0f; }),
                                menu_option("45", [&c](void* ptr) { c.fov = 45.0f; }),
                                menu_option("60", [&c](void* ptr) { c.fov = 60.0f; }),
                                menu_option("75", [&c](void* ptr) { c.fov = 75.0f; }),
                                menu_option("90", [&c](void* ptr) { c.fov = 90.0f; }),
                            }
                        );
                    }),
                    menu_option("Tracking", [&w](void* ptr){
                        menu* m = reinterpret_cast<menu*>(ptr);
                        m->set_submenu(
                            "Track",
                            {
                                menu_option("Nothing", [&w](void* ptr) {
                                    w.track_target = {track_none, 0};
                                    w.status("Tracking disabled, camera is now free");
                                }),
                                menu_option("Mario", [&w](void* ptr) {
                                    w.track_target = {track_mario, 0};
                                    w.status("Now tracking Mario's position");
                                }),
                                menu_option("Game camera", [&w](void* ptr) {
                                    w.track_target = {track_game_camera, 0};
                                    w.status("Now tracking game camera position");
                                }),
                                menu_option("Dynamic object", [&w](void* ptr) {
                                    menu* m = reinterpret_cast<menu*>(ptr);
                                    std::vector<menu_option> options(65);

                                    for (long i = 0; i < 65; i++)
                                        options[i] = menu_option(std::to_string(i), [&w, i](void* ptr) {
                                            w.track_target = {track_dynamic_object, i};
                                            w.status("Now tracking dynamic object " + std::to_string(i));
                                        });

                                    m->set_submenu("Which ID?", options);
                                }),
                                menu_option("Item object", [&w](void* ptr) {
                                    menu* m = reinterpret_cast<menu*>(ptr);
                                    std::vector<menu_option> options(65);

                                    for (long i = 0; i < 65; i++)
                                        options[i] = menu_option(std::to_string(i), [&w, i](void* ptr) {
                                            w.track_target = {track_item_object, i};
                                            w.status("Now tracking item object " + std::to_string(i));
                                        });

                                    m->set_submenu("Which ID?", options);
                                }),
                                menu_option("World object", [&w](void* ptr) {
                                    menu* m = reinterpret_cast<menu*>(ptr);
                                    std::vector<menu_option> options(32);

                                    for (long i = 0; i < 32; i++)
                                        options[i] = menu_option(std::to_string(i), [&w, i](void* ptr) {
                                            w.track_target = {track_world_object, i};
                                            w.status("Now tracking world object " + std::to_string(i));
                                        });

                                    m->set_submenu("Which ID?", options);
                                }),
                            }
                        );
                    }),
                    menu_option("Framedump", [&w](void* ptr){
                        menu* m = reinterpret_cast<menu*>(ptr);
                        m->set_submenu(
                            "Framedump settings",
                            {
                                menu_option("Record with UI", [&w](void* ptr) {
                                    w.recording_has_ui = true;
                                    w.status("Framedumps will now include UI");
                                }),
                                menu_option("Record without UI", [&w](void* ptr) {
                                    w.recording_has_ui = false;
                                    w.status("Framedumps will now NOT include UI");
                                }),
                            }
                        );
                    }),
                    menu_option("Toggle showing FPS", [&w](void* ptr) {
                        w.show_fps = !w.show_fps;
                    }),
                    menu_option("Exit", [&w](void* ptr) { w.close(); }),
                },
                [&w]() { w.active_menu.reset(); }
            ));

            w.status("Press [Backspace] to close the menu");

            return true;
        } else {
            return false;
        }
    } else {
        menu* menu_head = w.active_menu->get_head();

        if (w.input_state->keyboard[GLFW_KEY_DOWN].action == input::down)
            ++*menu_head;
        if (w.input_state->keyboard[GLFW_KEY_UP].action == input::down)
            --*menu_head;
        if (w.input_state->keyboard[GLFW_KEY_BACKSPACE].action == input::down)
            menu_head->back();
        if (w.input_state->keyboard[GLFW_KEY_ENTER].action == input::down)
            menu_head->select();

        return true;
    }
}

} // controls
} // eldstar
