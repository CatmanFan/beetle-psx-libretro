#include "texture.h"

#include <stdlib.h>
#include <assert.h>

Texture::Texture(uint32_t width, uint32_t height, GLenum internal_format)
{
    GLuint id = 0;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexStorage2D(GL_TEXTURE_2D,
                    1,
                    internal_format,
                    (GLsizei) width,
                    (GLsizei) height);

    assert( !glGetError() );


    this->id = id;
    this->width = width;
    this->height = height;
}

void Texture::bind(GLenum texture_unit)
{
    glActiveTexture(texture_unit);
    glBindTexture(GL_TEXTURE_2D, this->id);
}

void Texture::set_sub_image(uint16_t top_left[2],
                            uint16_t resolution[2],
                            GLenum format,
                            GLenum ty,
                            uint16_t* data)
{
    // if data.len() != (resolution.0 as usize * resolution.1 as usize) {
    //     panic!("Invalid texture sub_image size");
    // }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    (GLint) top_left[0],
                    (GLint) top_left[1],
                    (GLsizei) resolution[0],
                    (GLsizei) resolution[1],
                    format,
                    ty,
                    (void*) data);

    assert( !glGetError() );
}

void Texture::set_sub_image_window( uint16_t top_left[2],
                                    uint16_t resolution[2],
                                    size_t row_len,
                                    GLenum format,
                                    GLenum ty,
                                    uint16_t* data)
{
   uint16_t x = top_left[0];
   uint16_t y = top_left[1];

   size_t index = ((size_t) y) * row_len + ((size_t) x);

   uint16_t* sub_data = &( data[index] );

   glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) row_len);

   this->set_sub_image(top_left, resolution, format, ty, sub_data);

   glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

void Texture::drop()
{
    glDeleteTextures(1, &this->id);
}

