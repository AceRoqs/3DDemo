#ifndef RENDER_H
#define RENDER_H

namespace Graphics
{
    struct Polygon;
}

void initialize_gl_constants();
void initialize_gl_world_data(const std::vector<struct Bitmap>& texture_list, const std::vector<struct Vector3f>& vertices, const std::vector<struct Vector2f>& texture_coords);
void draw_list(const std::vector<struct Graphics::Polygon>& poly_vector, const std::vector<struct Vector3f>& vertices, const std::vector<struct Vector3f>& vertices2, unsigned int patch_count, unsigned int patch_texture_id, const class Emitter& emitter, const struct Camera& camera);

#endif

