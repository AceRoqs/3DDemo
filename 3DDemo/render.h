#ifndef RENDER_H
#define RENDER_H

void initialize_gl_constants();
void initialize_gl_world_data(const std::vector<struct Position_vertex>& vertex_formats, const std::vector<struct TexCoord>& texture_coords);
void draw_list(std::function<void(void)> swap_buffers, const std::vector<class CPolygon>& poly_vector, float camera_x, float camera_y, float camera_z, float camera_degrees);

#endif

