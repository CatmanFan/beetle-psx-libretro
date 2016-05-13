#ifndef RETROGL_BUFFER_H
#define RETROGL_BUFFER_H

#include "vertex.h"
#include "program.h"
#include "error.h"

#include <glsm/glsmsym.h>

#include <stdlib.h> // size_t
#include <stdint.h>
#include <assert.h>

#include <vector>

template<typename T>
class DrawBuffer 
{
public:
    /// OpenGL name for this buffer
    GLuint id;
    /// Vertex Array Object containing the bindings for this
    /// buffer. I'm assuming that each VAO will only use a single
    /// buffer for simplicity.
    VertexArrayObject* vao;
    /// Program used to draw this buffer
    Program* program;
    /// Number of elements T that the vertex buffer can hold
    size_t capacity;
    /// Marker for the type of our buffer's contents
    /* PhantomData<T> contains; */
    T* contains;
    /// Current number of entries in the buffer
    size_t len;
    /// If true newer items are added *before* older ones
    /// (i.e. they'll be drawn first)
    bool lifo;

    /* 
    pub fn new(capacity: usize,
               program: Program,
               lifo: bool) -> Result<DrawBuffer<T>, Error> {
    */
    DrawBuffer(size_t capacity, Program* program, bool lifo)
    {
        VertexArrayObject* vao = new VertexArrayObject();

        GLuint id = 0;
        // Generate the buffer object
        glGenBuffers(1, &id);

        this->vao = vao;
        this->program = program;
        this->capacity = capacity;
        this->id = id;
        
        this->contains = new T;

        this->clear();
        this->bind_attributes();

        /* error_or() */
        get_error();
    }

    ~DrawBuffer()
    {
        if (this->vao != nullptr)       delete vao;
        if (this->program != nullptr)   delete program;
        if (this->contains != nullptr)  delete contains;
        this->drop();
    }

    /* fn bind_attributes(&self)-> Result<(), Error> { */
    void bind_attributes()
    {
        printf("bind_attributes()\n");
        this->vao->bind();

        // ARRAY_BUFFER is captured by VertexAttribPointer
        this->bind();

        std::vector<Attribute> attrs = attributes(this->contains);
        printf("Contents of std::vector<Attribute> attrs: \n");
        for (Attribute attr : attrs) {
            printf("\t%s\n", attr.name.c_str());
        }

        GLint element_size = (GLint) sizeof( T );

        /* 
        let index =
                    match self.program.find_attribute(attr.name) {
                        Ok(i) => i,
                        // Don't error out if the shader doesn't use this
                        // attribute, it could be caused by shader
                        // optimization if the attribute is unused for
                        // some reason.
                        Err(Error::InvalidValue) => continue,
                        Err(e) => return Err(e),
                    };

        */
        for (Attribute attr : attrs) {
            GLint index = this->program->find_attribute(attr.name.c_str());

            // Don't error out if the shader doesn't use this
            // attribute, it could be caused by shader
            // optimization if the attribute is unused for
            // some reason.
            if (index < 0) {
                continue;
            }
            glEnableVertexAttribArray((GLuint) index);

            // This captures the buffer so that we don't have to bind it
            // when we draw later on, we'll just have to bind the vao
            switch (attr.ty) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
                glVertexAttribIPointer( index,
                                        attr.components,
                                        attr.ty,
                                        element_size,
                                        attr.gl_offset());
                break;
            case GL_FLOAT:
                glVertexAttribPointer(  index,
                                        attr.components,
                                        attr.ty,
                                        GL_FALSE,
                                        element_size,
                                        attr.gl_offset());
                break;
            case GL_DOUBLE:
                glVertexAttribLPointer( index,
                                        attr.components,
                                        attr.ty,
                                        element_size,
                                        attr.gl_offset());
                break;
            }
        }

        get_error();
    }

    void enable_attribute(const char* attr)
    {
        GLuint index = this->program->find_attribute(attr);
        this->vao->bind();

        glEnableVertexAttribArray(index);

        get_error();
    }

    void disable_attribute(const char* attr)
    {
        GLuint index = this->program->find_attribute(attr);
        this->vao->bind();

        glDisableVertexAttribArray(index);

        get_error();
    }

    bool empty()
    {
        return this->len == 0;
    }

    /* impl<T> DrawBuffer<T> { */

    /// Orphan the buffer (to avoid synchronization) and allocate a
    /// new one.
    ///
    /// https://www.opengl.org/wiki/Buffer_Object_Streaming
    void clear()
    {
        this->bind();

        size_t element_size = sizeof( *(this->contains) );
        GLsizeiptr storage_size = (GLsizeiptr) (this->capacity * element_size);
        glBufferData(   GL_ARRAY_BUFFER,
                        storage_size,
                        NULL,
                        GL_DYNAMIC_DRAW);

        this->len = 0;

        get_error();
    }

    /// Bind the buffer to the current VAO
    void bind()
    {
        glBindBuffer(GL_ARRAY_BUFFER, this->id);
    }

    void push_slice(T slice[], size_t n)
    {
        if (n > this->remaining_capacity() ) {
            puts("DrawBuffer::push_slice() - Out of memory\n");
            return;
        }

        size_t element_size = sizeof( *(this->contains) );

        size_t offset;
        if (this->lifo) {
            offset = this->capacity - this->len - n;
        } else {
            offset = this->len;
        }

        size_t offset_bytes = offset * element_size;
        size_t size_bytes = n * element_size;

        this->bind();

        glBufferSubData(GL_ARRAY_BUFFER,
                        (GLintptr) offset_bytes,
                        (GLintptr) size_bytes,
                        (void*) &slice);

        get_error();
        
        this->len += n;
    }

    void draw(GLenum mode)
    {
        this->vao->bind();
        this->program->bind();

        GLint first = this->lifo ? (GLint) this->remaining_capacity() : 0;

        glDrawArrays(mode, first, (GLsizei) this->len);
    }
    
    size_t remaining_capacity()
    {
        return this->capacity - this->len;
    }

    /* impl<T> Drop for DrawBuffer<T> { */
    void drop()
    {
        glDeleteBuffers(1, &this->id);
    }
};

#endif
