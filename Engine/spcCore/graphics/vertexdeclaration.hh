

#pragma once

#include <spcCore/coreexport.hh>
#include <spcCore/types.hh>
#include <spcCore/graphics/datatype.hh>
#include <spcCore/graphics/vertexstream.hh>
#include <vector>

namespace spc
{



class SPC_CORE_API VertexDeclaration
{
public:
  struct Attribute 
  {
    UInt8 Stream;
    eVertexStream Location;
    UInt8 Size;
    eDataType Type;
    UInt16 Stride;
    UInt16 Offset;
    Attribute() {}
    Attribute(UInt8 stream, eVertexStream location, UInt8 size, eDataType type, UInt16 stride, UInt16 offset)
      : Stream(stream)
      , Location(location)
      , Size(size)
      , Type(type)
      , Stride(stride)
      , Offset(offset)
    {}
  };



  VertexDeclaration(const Attribute* attributes, Size count);
  VertexDeclaration(const std::vector<Attribute>& attributes);

  const std::vector<Attribute>& GetAttributes(UInt8 streamID) const;

private:
  void Init(const Attribute* attributes, Size count);
  struct Stream 
  {
    UInt8 StreamID;
    std::vector<Attribute> Attributes;
  };

  std::vector<Stream> m_streams;
};


}