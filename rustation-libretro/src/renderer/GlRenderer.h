
#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include <glsm/glsmsym.h>
#include <vector>
#include <cstdio>
#include <stdint.h>

#include "libretro.h"

#include "../retrogl/buffer.h"
#include "../retrogl/shader.h"
#include "../retrogl/program.h"
#include "../retrogl/texture.h"
#include "../retrogl/framebuffer.h"
#include "../retrogl/error.h"

#ifdef __cplusplus
extern "C"
{
#endif
	extern retro_environment_t environ_cb;
	extern retro_video_refresh_t video_cb;
#ifdef __cplusplus
}
#endif

const uint16_t VRAM_WIDTH_PIXELS = 1024;
const uint16_t VRAM_HEIGHT = 512;
const size_t VRAM_PIXELS = (size_t) VRAM_WIDTH_PIXELS * (size_t) VRAM_HEIGHT;

/// How many vertices we buffer before forcing a draw
static const unsigned int VERTEX_BUFFER_LEN = 0x8000;
/// Maximum number of indices for a vertex buffer. Since quads have
/// two duplicated vertices it can be up to 3/2 the vertex buffer
/// length
static const unsigned int INDEX_BUFFER_LEN = ((VERTEX_BUFFER_LEN * 3 + 1) / 2);

struct DrawConfig {
    uint16_t display_top_left[2];
    uint16_t display_resolution[2];
    bool     display_24bpp;
    bool     display_off;
    int16_t  draw_offset[2];
    uint16_t draw_area_top_left[2];
    uint16_t draw_area_bot_right[2];
};

struct CommandVertex {
    /// Position in PlayStation VRAM coordinates
    float position[4];
    /// RGB color, 8bits per component
    uint8_t color[3];
    /// Texture coordinates within the page
    uint16_t texture_coord[2];
    /// Texture page (base offset in VRAM used for texture lookup)
    uint16_t texture_page[2];
    /// Color Look-Up Table (palette) coordinates in VRAM
    uint16_t clut[2];
    /// Blending mode: 0: no texture, 1: raw-texture, 2: texture-blended
    uint8_t texture_blend_mode;
    /// Right shift from 16bits: 0 for 16bpp textures, 1 for 8bpp, 2
    /// for 4bpp
    uint8_t depth_shift;
    /// True if dithering is enabled for this primitive
    uint8_t dither;
    /// 0: primitive is opaque, 1: primitive is semi-transparent
    uint8_t semi_transparent;
    /// Texture window mask/OR values
    uint8_t texture_window[4];

    static std::vector<Attribute> attributes();
};

struct OutputVertex {
    /// Vertex position on the screen
    float position[2];
    /// Corresponding coordinate in the framebuffer
    uint16_t fb_coord[2];

    static std::vector<Attribute> attributes();
};

struct ImageLoadVertex {
    // Vertex position in VRAM
    uint16_t position[2];

    static std::vector<Attribute> attributes();
};

enum SemiTransparencyMode {
    /// Source / 2 + destination / 2
    SemiTransparencyMode_Average = 0,
    /// Source + destination
    SemiTransparencyMode_Add = 1,
    /// Destination - source
    SemiTransparencyMode_SubtractSource = 2,
    /// Destination + source / 4
    SemiTransparencyMode_AddQuarterSource = 3,
};

struct TransparencyIndex {
    SemiTransparencyMode transparency_mode;
    unsigned last_index;
    GLenum draw_mode;

  TransparencyIndex(SemiTransparencyMode transparency_mode,
		    unsigned last_index,
		    GLenum draw_mode)
    :transparency_mode(transparency_mode),
     last_index(last_index),
     draw_mode(draw_mode)
  {
  }
};

class GlRenderer {
public:
    /// Buffer used to handle PlayStation GPU draw commands
    DrawBuffer<CommandVertex>* command_buffer;
    GLushort opaque_triangle_indices[INDEX_BUFFER_LEN];
    GLushort opaque_line_indices[INDEX_BUFFER_LEN];
    GLushort semi_transparent_indices[INDEX_BUFFER_LEN];
    /// Primitive type for the vertices in the command buffers
    /// (TRIANGLES or LINES)
    GLenum command_draw_mode;
    unsigned opaque_triangle_index_pos;
    unsigned opaque_line_index_pos;
    unsigned semi_transparent_index_pos;
    /// Current semi-transparency mode
    SemiTransparencyMode semi_transparency_mode;
    std::vector<TransparencyIndex> transparency_mode_index;
    /// Polygon mode (for wireframe)
    GLenum command_polygon_mode;
    /// Buffer used to draw to the frontend's framebuffer
    DrawBuffer<OutputVertex>* output_buffer;
    /// Buffer used to copy textures from `fb_texture` to `fb_out`
    DrawBuffer<ImageLoadVertex>* image_load_buffer;
    /// Texture used to store the VRAM for texture mapping
    DrawConfig* config;
    /// Framebuffer used as a shader input for texturing draw commands
    Texture* fb_texture;
    /// Framebuffer used as an output when running draw commands
    Texture* fb_out;
    /// Depth buffer for fb_out
    Texture* fb_out_depth;
    /// Current resolution of the frontend's framebuffer
    uint32_t frontend_resolution[2];
    /// Current internal resolution upscaling factor
    uint32_t internal_upscaling;
    /// Current internal color depth
    uint8_t internal_color_depth;
    /// Counter for preserving primitive draw order in the z-buffer
    /// since we draw semi-transparent primitives out-of-order.
    int16_t primitive_ordering;
    /// Texture window mask/OR values
    uint8_t tex_x_mask;
    uint8_t tex_x_or;
    uint8_t tex_y_mask;
    uint8_t tex_y_or;

    uint32_t mask_set_or;
    uint32_t mask_eval_and;

    uint8_t filter_type;

    /// When true we display the entire VRAM buffer instead of just
    /// the visible area
    bool display_vram;

    /* pub fn from_config(config: DrawConfig) -> Result<GlRenderer, Error> */
    GlRenderer(DrawConfig* config);

    ~GlRenderer();

    template<typename T>
    static DrawBuffer<T>* build_buffer( const char* vertex_shader,
                                        const char* fragment_shader,
                                        size_t capacity)
    {
        Shader* vs = new Shader(vertex_shader, GL_VERTEX_SHADER);
        Shader* fs = new Shader(fragment_shader, GL_FRAGMENT_SHADER);
        Program* program = new Program(vs, fs);

        return new DrawBuffer<T>(capacity, program);
    }

    void draw();
    void apply_scissor();
    void bind_libretro_framebuffer();
    void upload_textures(   uint16_t top_left[2],
                            uint16_t dimensions[2],
                            uint16_t pixel_buffer[VRAM_PIXELS]);

    void upload_vram_window(uint16_t top_left[2],
                            uint16_t dimensions[2],
                            uint16_t pixel_buffer[VRAM_PIXELS]);

    DrawConfig* draw_config();
    void prepare_render();
    bool refresh_variables();
    void finalize_frame();

    void set_mask_setting(uint32_t mask_set_or, uint32_t mask_eval_and);
    void set_draw_offset(int16_t x, int16_t y);
    void set_draw_area(uint16_t top_left[2], uint16_t bot_right[2]);
    void set_tex_window(uint8_t tww, uint8_t twh, uint8_t twx,
          uint8_t twy);

    void set_display_mode(  uint16_t top_left[2],
                            uint16_t resolution[2],
                            bool depth_24bpp);
    void set_display_off(bool off);

    void vertex_preprocessing(CommandVertex *v,
			      unsigned count,
			      GLenum mode,
			      SemiTransparencyMode stm);

    void push_quad(CommandVertex v[4],
		   SemiTransparencyMode semi_transparency_mode);

    void push_primitive(CommandVertex *v,
			unsigned count,
			GLenum mode,
			SemiTransparencyMode semi_transparency_mode);

    void push_triangle( CommandVertex v[3],
                        SemiTransparencyMode semi_transparency_mode);

    void push_line( CommandVertex v[2],
                    SemiTransparencyMode semi_transparency_mode);

    void fill_rect( uint8_t color[3],
                    uint16_t top_left[2],
                    uint16_t dimensions[2]);

    void copy_rect( uint16_t source_top_left[2],
                    uint16_t target_top_left[2],
                    uint16_t dimensions[2]);

    bool has_software_renderer();
};

std::vector<Attribute> attributes(CommandVertex* v);
std::vector<Attribute> attributes(OutputVertex* v);
std::vector<Attribute> attributes(ImageLoadVertex* v);

#endif
