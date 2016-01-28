
template<bool textured, int BlendMode, bool TexMult, uint32_t TexMode_TA, bool MaskEval_TA, bool FlipX, bool FlipY>
void PS_GPU::DrawSprite(int32_t x_arg, int32_t y_arg, int32_t w, int32_t h, uint8_t u_arg, uint8_t v_arg, uint32_t color, uint32_t clut_offset)
{
   const int32_t r = color & 0xFF;
   const int32_t g = (color >> 8) & 0xFF;
   const int32_t b = (color >> 16) & 0xFF;
   const uint16_t fill_color = 0x8000 | ((r >> 3) << 0) | ((g >> 3) << 5) | ((b >> 3) << 10);

   int32_t x_start, x_bound;
   int32_t y_start, y_bound;
   uint16_t u, v;
   int v_inc = 1, u_inc = 1;

   //printf("[GPU] Sprite: x=%d, y=%d, w=%d, h=%d\n", x_arg, y_arg, w, h);

   x_start = x_arg;
   x_bound = x_arg + w;

   y_start = y_arg;
   y_bound = y_arg + h;

   if(textured)
   {
      u = u_arg;
      v = v_arg;

      //if(FlipX || FlipY || (u & 1) || (v & 1) || ((TexMode_TA == 0) && ((u & 3) || (v & 3))))
      // fprintf(stderr, "Flippy: %d %d 0x%02x 0x%02x\n", FlipX, FlipY, u, v);

      if(FlipX)
      {
         u_inc = -1;
         u |= 1;
      }
      // FIXME: Something weird happens when lower bit of u is set and we're not doing horizontal flip, but I'm not sure what it is exactly(needs testing)
      // It may only happen to the first pixel, so look for that case too during testing.
      //else
      // u = (u + 1) & ~1;

      if(FlipY)
      {
         v_inc = -1;
      }
   }

   if(x_start < ClipX0)
   {
      if(textured)
         u += (ClipX0 - x_start) * u_inc;

      x_start = ClipX0;
   }

   if(y_start < ClipY0)
   {
      if(textured)
         v += (ClipY0 - y_start) * v_inc;

      y_start = ClipY0;
   }

   if(x_bound > (ClipX1 + 1))
      x_bound = ClipX1 + 1;

   if(y_bound > (ClipY1 + 1))
      y_bound = ClipY1 + 1;


   // Upscale
   y_start <<= UPSCALE_SHIFT;
   y_bound <<= UPSCALE_SHIFT;
   x_start <<= UPSCALE_SHIFT;
   x_bound <<= UPSCALE_SHIFT;
   u = (u & 0xff) << UPSCALE_SHIFT;
   v = (v & 0xff) << UPSCALE_SHIFT;

   //HeightMode && !dfe && ((y & 1) == ((DisplayFB_YStart + !field_atvs) & 1)) && !DisplayOff
   //printf("%d:%d, %d, %d ---- heightmode=%d displayfb_ystart=%d field_atvs=%d displayoff=%d\n", w, h, scanline, dfe, HeightMode, DisplayFB_YStart, field_atvs, DisplayOff);

   for(int32_t y = y_start; MDFN_LIKELY(y < y_bound); y++)
   {
      uint16_t u_r;

      if(textured)
         u_r = u;

      if(!LineSkipTest(this, y >> UPSCALE_SHIFT))
      {
         if(y_bound > y_start && x_bound > x_start)
         {
            // Note(TODO): From tests on a PS1, even a 0-width sprite takes up time to "draw" proportional to its height.
            int32_t suck_time = /* 8 + */ (x_bound - x_start);

            if((BlendMode >= 0) || MaskEval_TA)
               suck_time += (((x_bound + 1) & ~1) - (x_start & ~1)) >> 1;

            DrawTimeAvail -= suck_time >> UPSCALE_SHIFT;
         }

         for(int32_t x = x_start; MDFN_LIKELY(x < x_bound); x++)
         {
            if(textured)
            {
	      uint16_t fbw = GetTexel<TexMode_TA>(clut_offset,
						  u_r >> UPSCALE_SHIFT,
						  v >> UPSCALE_SHIFT);

               if(fbw)
               {
                  if(TexMult)
                  {
                     fbw = ModTexel(fbw, r, g, b, 3, 2);
                  }
                  PlotPixel<BlendMode, MaskEval_TA, true>(x, y, fbw);
               }
            }
            else
               PlotPixel<BlendMode, MaskEval_TA, false>(x, y, fill_color);

            if(textured)
               u_r += u_inc;
         }
      }
      if(textured)
         v += v_inc;
   }
}

template<uint8_t raw_size, bool textured, int BlendMode, bool TexMult, uint32_t TexMode_TA, bool MaskEval_TA>
INLINE void PS_GPU::Command_DrawSprite(const uint32_t *cb)
{
   int32_t x, y;
   int32_t w, h;
   uint8_t u = 0, v = 0;
   uint32_t color = 0;
   uint32_t clut = 0;

   DrawTimeAvail -= 16;	// FIXME, correct time.

   color = *cb & 0x00FFFFFF;
   cb++;

   x = sign_x_to_s32(11, (*cb & 0xFFFF));
   y = sign_x_to_s32(11, (*cb >> 16));
   cb++;

   if(textured)
   {
      u = *cb & 0xFF;
      v = (*cb >> 8) & 0xFF;
      clut = ((*cb >> 16) & 0xFFFF) << 4;
      Update_CLUT_Cache<TexMode_TA>((*cb >> 16) & 0xFFFF);
      cb++;
   }

   switch(raw_size)
   {
      default:
      case 0:
         w = (*cb & 0x3FF);
         h = (*cb >> 16) & 0x1FF;
         cb++;
         break;

      case 1:
         w = 1;
         h = 1;
         break;

      case 2:
         w = 8;
         h = 8;
         break;

      case 3:
         w = 16;
         h = 16;
         break;
   }

   //printf("SPRITE: %d %d %d -- %d %d\n", raw_size, x, y, w, h);

   x = sign_x_to_s32(11, x + OffsX);
   y = sign_x_to_s32(11, y + OffsY);

   switch(SpriteFlip & 0x3000)
   {
      case 0x0000:
         if(!TexMult || color == 0x808080)
            DrawSprite<textured, BlendMode, false, TexMode_TA, MaskEval_TA, false, false>(x, y, w, h, u, v, color, clut);
         else
            DrawSprite<textured, BlendMode, true, TexMode_TA, MaskEval_TA, false, false>(x, y, w, h, u, v, color, clut);
         break;

      case 0x1000:
         if(!TexMult || color == 0x808080)
            DrawSprite<textured, BlendMode, false, TexMode_TA, MaskEval_TA, true, false>(x, y, w, h, u, v, color, clut);
         else
            DrawSprite<textured, BlendMode, true, TexMode_TA, MaskEval_TA, true, false>(x, y, w, h, u, v, color, clut);
         break;

      case 0x2000:
         if(!TexMult || color == 0x808080)
            DrawSprite<textured, BlendMode, false, TexMode_TA, MaskEval_TA, false, true>(x, y, w, h, u, v, color, clut);
         else
            DrawSprite<textured, BlendMode, true, TexMode_TA, MaskEval_TA, false, true>(x, y, w, h, u, v, color, clut);
         break;

      case 0x3000:
         if(!TexMult || color == 0x808080)
            DrawSprite<textured, BlendMode, false, TexMode_TA, MaskEval_TA, true, true>(x, y, w, h, u, v, color, clut);
         else
            DrawSprite<textured, BlendMode, true, TexMode_TA, MaskEval_TA, true, true>(x, y, w, h, u, v, color, clut);
         break;
   }
}
