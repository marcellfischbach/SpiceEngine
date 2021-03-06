#include <spcCore/settings.hh>
#include <spcCore/resource/file.hh>
#include <spcCore/resource/resourcelocator.hh>
#include <spcCore/resource/vfs.hh>
#

namespace spc
{

Settings::Settings(const std::string& locator)
        : m_file(nullptr)
{
  Initialize(locator);
}

Settings::Settings(const ResourceLocator& locator)
        : m_file(nullptr)
{
  Initialize(locator);
}

void Settings::Initialize(const ResourceLocator& locator)
{
  iFile* file = VFS::Get()->Open(locator, eAM_Read, eOM_Text);
  if (file)
  {
    m_file = new file::File();
    if (!m_file->Parse(file))
    {
      delete m_file;
      m_file = nullptr;
    }
    file->Release();
  }
}

Settings::~Settings()
{
  delete m_file;
  m_file = nullptr;
}

bool Settings::IsValid() const
{
  return m_file;
}

std::string Settings::GetText(const std::string& path, const std::string& defaultValue) const
{
  file::Element* element = GetElement(path);
  if (element)
  {
    return element->GetAttribute(0, "");
  }
  return defaultValue;
}

float Settings::GetFloat(const std::string& path, float defaultValue) const
{
  file::Element* element = GetElement(path);
  if (element)
  {
    return (float) atof(element->GetAttribute(0, "").c_str());
  }
  return defaultValue;
}

int Settings::GetInt(const std::string& path, int defaultValue) const
{
  file::Element* element = GetElement(path);
  if (element)
  {
    return atoi(element->GetAttribute(0, "").c_str());
  }
  return defaultValue;
}

bool Settings::GetBool(const std::string& path, bool defaultValue) const
{
  std::string v = GetText(path, defaultValue ? "true" : "false");
  return std::string("true") == v
         || std::string("on") == v
         || std::string("yes") == v
         || std::string("positiv") == v
         || std::string("1") == v;
}

Vector2f Settings::GetVector2f(const std::string& path, const Vector2f& defaultValue) const
{
  file::Element* element = GetElement(path);
  if (element && element->GetNumberOfAttributes() >= 2)
  {
    return Vector2f(
            (float) element->GetAttribute(0)->GetDoubleValue(),
            (float) element->GetAttribute(1)->GetDoubleValue()
    );
  }
  return defaultValue;
}

Vector3f Settings::GetVector3f(const std::string& path, const Vector3f& defaultValue) const
{
  file::Element* element = GetElement(path);
  if (element && element->GetNumberOfAttributes() >= 3)
  {
    return Vector3f(
            (float) element->GetAttribute(0)->GetDoubleValue(),
            (float) element->GetAttribute(1)->GetDoubleValue(),
            (float) element->GetAttribute(2)->GetDoubleValue()
    );
  }
  return defaultValue;
}


Vector4f Settings::GetVector4f(const std::string& path, const Vector4f& defaultValue) const
{
  file::Element* element = GetElement(path);
  if (element && element->GetNumberOfAttributes() >= 4)
  {
    return Vector4f(
            (float) element->GetAttribute(0)->GetDoubleValue(),
            (float) element->GetAttribute(1)->GetDoubleValue(),
            (float) element->GetAttribute(2)->GetDoubleValue(),
            (float) element->GetAttribute(3)->GetDoubleValue()
    );
  }
  return defaultValue;
}


Vector2i Settings::GetVector2i(const std::string& path, const Vector2i& defaultValue) const
{
  file::Element* element = GetElement(path);
  if (element && element->GetNumberOfAttributes() >= 2)
  {
    return Vector2i(
            element->GetAttribute(0)->GetIntValue(),
            element->GetAttribute(1)->GetIntValue()
    );
  }
  return defaultValue;
}
file::Element* Settings::GetElement(const std::string& path) const
{
  return m_file ? m_file->Root()->GetChild(path) : nullptr;
}

}