
#include <spcOpenGL/gl4/gl4indexbuffer.hh>
#include <gl/glew.h>


namespace spc::opengl
{

GL4IndexBuffer::GL4IndexBuffer()
  : iIndexBuffer()
{
  SPC_CLASS_GEN_CONSTR;
  glGenBuffers(1, &m_name);
}

GL4IndexBuffer::~GL4IndexBuffer()
{
  if (m_name != 0)
  {
    glDeleteBuffers(1, &m_name);
    m_name = 0;
  }
}

void GL4IndexBuffer::CreateForRendering(Size size, eBufferUsage usage)
{
  GLenum mode;
  switch (usage)
  {
  case eBU_Static: mode = GL_STATIC_DRAW; break;
  case eBU_Stream: mode = GL_STREAM_DRAW; break;
  case eBU_Dynamic: mode = GL_DYNAMIC_DRAW; break;
  }
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, nullptr, mode);
}

void GL4IndexBuffer::Bind()
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name);
}

void GL4IndexBuffer::Unbind()
{
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GL4IndexBuffer::Copy(const void* data, Size count, Size targetOffset)
{
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, targetOffset, count, data);
}

}