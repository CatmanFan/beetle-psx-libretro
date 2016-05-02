#include "texture.h"

#include <stdlib.h>
#include <stdio.h>

Texture::Texture(uint32_t width, uint32_t height, GLenum internal_format)
{
    GLuint id = 0;

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexStorage2D( GL_TEXTURE_2D,
                    1,
                    internal_format,
                    (GLsizei) width,
                    (GLsizei) height);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        printf("GL error %d", (int) error);
        exit(EXIT_FAILURE);
    }

    this->id = id;
    this->width = width;
    this->height = height;
}

void Texture::bind(GLenum texture_unit)
{
    glActiveTexture(texture_unit);
    glBindTexture(GL_TEXTURE_2D, this->id);
}

GLenum Texture::set_sub_image(   uint16_t top_left[2],
                        uint16_t resolution[2],
                        GLenum format,
                        GLenum ty,
                        uint16_t* data)
{
    // if data.len() != (resolution.0 as usize * resolution.1 as usize) {
    //     panic!("Invalid texture sub_image size");
    // }

    glPixelStoragei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, this->id);
    glTexSubImage2D(GL_TEXTURE_2D,
                    0,
                    (GLint) top_left[0],
                    (GLint) top_left[1],
                    (GLsizei) resolution[0],
                    (GLsizei) resolution[1],
                    format,
                    ty,
                    (const void*) data);

    return glGetError();
}

GLenum Texture::set_sub_image_window(uint16_t top_left[2],
                            uint16_t resolution[2],
                            size_t row_len,
                            GLenum format,
                            GLenum ty,
                            uint16_t* data)
{
   uint16_t x = top_left[0];
   uint16_t y = top_left[1];

   size_t index = ((size_t) y) * row_len + ((size_t) x);

   uint16_t data = data[index];

   glPixelStoragei(GL_UNPACK_ROW_LENGTH, (GLint) row_len);

   GLenum error = this->set_sub_image(top_left, resolution, format, ty, data);

   glPixelStoragei(GL_UNPACK_ROW_LENGTH, 0);

   return error;
}

GLuint Texture::id()
{
    return this->id;
}

uint32_t Texture::width()
{
    this->width;
}

uint32_t Texture::height()
{
    this->height;
}

void Texture::drop()
{
    glDeleteTextures(1, &this->id);
}


