#include <spcCore/entity/entitystate.hh>
#include <spcCore/entity/entity.hh>
#include <spcCore/entity/world.hh>
#include <entity/entitystate.refl.hh>

namespace spc
{

EntityState::EntityState(const std::string &name)
  : iObject()
  , m_name(name)
  , m_needUpdate(false)
  , m_entity(nullptr)
{
  SPC_CLASS_GEN_CONSTR;
}

EntityState::~EntityState()
{

}

void EntityState::SetName(const std::string &name)
{
  m_name = name;
}

const std::string &EntityState::GetName() const
{
  return m_name;
}

void EntityState::SetEntity(Entity *entity)
{
  if (m_entity != entity)
  {
    Entity* oldEntity = m_entity;
    m_entity = entity;
    
    UpdateEntity(oldEntity, m_entity);
    
    SPC_ADDREF(m_entity);
    SPC_RELEASE(oldEntity);
  }
}

Entity* EntityState::GetEntity()
{
  return m_entity;
}


const Entity* EntityState::GetEntity() const
{
  return m_entity;
}

World* EntityState::GetWorld()
{
  return m_entity ? m_entity->GetWorld() : nullptr;
}

const World* EntityState::GetWorld() const
{
  return m_entity ? m_entity->GetWorld() : nullptr;
}

void EntityState::SetNeedUpdate(bool needUpdate)
{
  if (m_needUpdate != needUpdate)
  {
    World* world = GetWorld();
    if (world && m_needUpdate)
    {
      world->DetachUpdateState(this);
    }
    m_needUpdate = needUpdate;
    if (world && m_needUpdate)
    {
      world->AttachUpdateState(this);
    }
  }
}

bool EntityState::IsNeedUpdate() const
{
  return m_needUpdate;
}

void EntityState::UpdateEntity(Entity *oldEntity, Entity *newEntity)
{
  if (oldEntity)
  {
    oldEntity->DeregisterEntityState(this);
  }
  if (newEntity)
  {
    newEntity->RegisterEntityState(this);
  }
}

void EntityState::AttachToWorld(World *world)
{
  if (m_needUpdate)
  {
    world->AttachUpdateState(this);
  }
  OnAttachedToWorld(world);
}

void EntityState::DetachFromWorld(World *world)
{
  if (m_needUpdate)
  {
    world->DetachUpdateState(this);
  }
  OnAttachedToWorld(world);
}


void EntityState::OnAttachedToWorld(World* world)
{

}

void EntityState::OnDetachedFromWorld(World* world)
{

}

void EntityState::Update(float tpf)
{
  // nothing to be done
}

}
