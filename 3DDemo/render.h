#ifndef RENDER_H
#define RENDER_H

namespace Graphics
{
    struct Polygon;
}

void initialize_gl_constants();
void initialize_gl_world_data(const std::vector<struct Bitmap>& texture_list, const std::vector<struct Position_vertex>& vertex_formats, const std::vector<struct TexCoord>& texture_coords);
void draw_list(const std::vector<struct Graphics::Polygon>& poly_vector, const class CEmitter& emitter, const struct Camera& camera);

#endif

