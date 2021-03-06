#pragma once

#include "loaders/file.hpp"
#include "loaders/png.hpp"
#include "loaders/shader.hpp"
#include "loaders/stl.hpp"
#include "loaders/truetype.hpp"


namespace eldstar {

class resource_manager {
    public:
        resource_manager() :
            freetype(),
            opensans(&freetype, loaders::file("resources/fonts/OpenSans.ttf"), 32),

            text({
                loaders::shader(GL_VERTEX_SHADER, loaders::file("resources/shaders/text_vs.glsl")),
                loaders::shader(GL_FRAGMENT_SHADER, loaders::file("resources/shaders/text_fs.glsl"))
            }),
            mesh({
                loaders::shader(GL_VERTEX_SHADER, loaders::file("resources/shaders/mesh_vs.glsl")),
                loaders::shader(GL_FRAGMENT_SHADER, loaders::file("resources/shaders/mesh_fs.glsl"))
            }),

            camera(loaders::file("resources/models/camera.stl")),
            cylinder_caps(loaders::file("resources/models/cylinder_caps.stl")),
            cylinder_perimeter(loaders::file("resources/models/cylinder_perimeter.stl")),
            cylinder_pointer(loaders::file("resources/models/cylinder_pointer.stl")),
            world_object(loaders::file("resources/models/world_object.stl")),

            #if defined(ELDSTAR_IS_TASSAFE)
            icon_128(loaders::file("resources/images/eldstar_tassafe_128.png")),
            icon_48(loaders::file("resources/images/eldstar_tassafe_48.png")),
            icon_32(loaders::file("resources/images/eldstar_tassafe_32.png")),
            icon_16(loaders::file("resources/images/eldstar_tassafe_16.png")),
            #elif defined(ELDSTAR_IS_CHEATS)
            icon_128(loaders::file("resources/images/eldstar_cheats_128.png")),
            icon_48(loaders::file("resources/images/eldstar_cheats_48.png")),
            icon_32(loaders::file("resources/images/eldstar_cheats_32.png")),
            icon_16(loaders::file("resources/images/eldstar_cheats_16.png")),
            #endif

            stub(1)
        {};

        loaders::text_manager freetype;
        loaders::truetype opensans;

        loaders::shader_program text;
        loaders::shader_program mesh;

        loaders::stl camera;
        loaders::stl cylinder_caps;
        loaders::stl cylinder_perimeter;
        loaders::stl cylinder_pointer;
        loaders::stl world_object;

        loaders::png icon_128;
        loaders::png icon_48;
        loaders::png icon_32;
        loaders::png icon_16;

        int stub;
};

} // eldstar
