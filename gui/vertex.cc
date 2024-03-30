#include "vertex.hh"

#include <GL/glew.h>
#include <cstdint>
#include <iostream>
#include <span>

namespace Gui {

Vertex::Vertex(std::span<float> vertex) : vertex_{vertex}
{
    glGenBuffers(1, &vertex_id_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_id_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertex_.size_bytes()),
                 vertex_.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
}

Vertex::Vertex(std::span<float> vertex, std::span<const uint32_t> indices)
    : vertex_{vertex}, index_{indices}
{
    glGenBuffers(1, &vertex_id_);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_id_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertex_.size_bytes()),
                 vertex_.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, nullptr);
    glGenBuffers(1, &index_id_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_id_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(index_.size_bytes()), index_.data(),
                 GL_STATIC_DRAW);
}

void
Vertex::draw(Vertex::Draw_command command)
{
    switch (command.mode)
    {
    case Primitive::triangle:
        glDrawArrays(GL_TRIANGLES, command.index, command.count);
        break;
    case Primitive::quad:
        glDrawElements(GL_TRIANGLES, command.count, GL_UNSIGNED_INT, nullptr);
        break;
    default:
        std::cerr << "unknown vertex draw command was issued to vertex.\n";
    };
}

Vertex::~Vertex()
{
    if (vertex_id_)
    {
        glDeleteBuffers(1, &vertex_id_);
    }
    if (index_id_)
    {
        glDeleteBuffers(1, &index_id_);
    }
}

} // namespace Gui
