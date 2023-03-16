#include "drawing/entity.h"
#include "drawing/components.h"

namespace medicimage
{
  entt::registry Entity::s_registry;

  Entity::Entity(entt::entity handle)
  	: m_entityHandle(handle)
  {
  }

  void Entity::DestroyEntity(Entity entity)
  {
    s_registry.destroy(entity);
  }

  Entity Entity::CreateEntity(int id, const std::string &name)
  {
		Entity entity = { s_registry.create()};
		entity.AddComponent<IDComponent>();
		entity.AddComponent<TransformComponent>();
    entity.AddComponent<CommonAttributesComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.tag = name.empty() ? "Entity" : name;

    return entity;
  }
  
} // namespace medicimage
