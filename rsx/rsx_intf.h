#ifndef __RSX_INTF_H__
#define __RSX_INTF_H__

#include "libretro.h"

#include "rsx.h"

#define MEDNAFEN_CORE_NAME_MODULE "psx"
#define MEDNAFEN_CORE_NAME "Mednafen PSX"
#define MEDNAFEN_CORE_VERSION "v0.9.38.6"
#define MEDNAFEN_CORE_EXTENSIONS "cue|toc|ccd|m3u|pbp"
#define MEDNAFEN_CORE_GEOMETRY_BASE_W 320
#define MEDNAFEN_CORE_GEOMETRY_BASE_H 240
#define MEDNAFEN_CORE_GEOMETRY_MAX_W 700
#define MEDNAFEN_CORE_GEOMETRY_MAX_H 576
#define MEDNAFEN_CORE_GEOMETRY_ASPECT_RATIO (4.0 / 3.0)

enum rsx_renderer_type
{
   RSX_SOFTWARE = 0,
   RSX_OPENGL,
   RSX_EXTERNAL_RUST
};

enum blending_modes
{
  BLEND_MODE_OPAQUE = -1,
  BLEND_MODE_AVERAGE = 0,
  BLEND_MODE_ADD = 1,
  BLEND_MODE_SUBTRACT = 2,
  BLEND_MODE_ADD_FOURTH = 3,
};

  void rsx_intf_set_blend_mode(enum blending_modes mode);

  void rsx_intf_set_environment(retro_environment_t cb);
  void rsx_intf_set_video_refresh(retro_video_refresh_t cb);
  void rsx_intf_get_system_av_info(struct retro_system_av_info *info);

  void rsx_intf_init(enum rsx_renderer_type type);
  bool rsx_intf_open(bool is_pal);
  void rsx_intf_close(void);
  void rsx_intf_refresh_variables(void);
  void rsx_intf_prepare_frame(void);
  void rsx_intf_finalize_frame(const void *data, unsigned width,
        unsigned height, unsigned pitch);

  void rsx_intf_set_draw_offset(int16_t x, int16_t y);
  void rsx_intf_set_draw_area(uint16_t x, uint16_t y,
                         uint16_t w, uint16_t h);
  void rsx_intf_set_display_mode(uint16_t x, uint16_t y,
                            uint16_t w, uint16_t h,
                            bool depth_24bpp);

  void rsx_intf_push_triangle(int16_t p0x, int16_t p0y,
                         int16_t p1x, int16_t p1y,
                         int16_t p2x, int16_t p2y,
                         uint32_t c0,
                         uint32_t c1,
                         uint32_t c2,
                         uint16_t t0x, uint16_t t0y,
                         uint16_t t1x, uint16_t t1y,
                         uint16_t t2x, uint16_t t2y,
                         uint16_t texpage_x, uint16_t texpage_y,
                         uint16_t clut_x, uint16_t clut_y,
                         uint8_t texture_blend_mode,
                         uint8_t depth_shift,
                         bool dither,
                         // This is really an `enum blending_modes`
                         // but I don't want to deal with enums in the
                         // FFI
                         int blend_mode);

  void rsx_intf_push_line(int16_t p0x, int16_t p0y,
                     int16_t p1x, int16_t p1y,
                     uint32_t c0,
                     uint32_t c1,
                     bool dither,
                     // This is really an `enum blending_modes`
                     // but I don't want to deal with enums in the
                     // FFI
                     int blend_mode);

  void rsx_intf_load_image(uint16_t x, uint16_t y,
		      uint16_t w, uint16_t h,
		      uint16_t *vram);

  void rsx_intf_fill_rect(uint32_t color,
		     uint16_t x, uint16_t y,
		     uint16_t w, uint16_t h);

  void rsx_intf_copy_rect(uint16_t src_x, uint16_t src_y,
		     uint16_t dst_x, uint16_t dst_y,
		     uint16_t w, uint16_t h);

  enum rsx_renderer_type rsx_intf_is_type(void);

extern retro_environment_t environ_cb;

#endif /*__RSX_H__ */
