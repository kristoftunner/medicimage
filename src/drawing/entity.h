#pragma once

#include <entt/entt.hpp>
#include "drawing/components.h"
#include "core/assert.h"
#include <string>
namespace medicimage
{
class Entity
{
public:
	Entity() = default;
	Entity(entt::entity handle);
	Entity(const Entity& other) = default;

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		MI_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");  
		T& component = s_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);
		//m_sheet->OnComponentAdded<T>(*this, component);
		return component;
	}

	template<typename T, typename... Args>
	T& AddOrReplaceComponent(Args&&... args)
	{
		T& component = s_registry.emplace_or_replace<T>(m_entityHandle, std::forward<Args>(args)...);
		//m_sheet->OnComponentAdded<T>(*this, component);
		return component;
	}

	template<typename T>
	T& GetComponent()
	{
		MI_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
		return s_registry.get<T>(m_entityHandle);
	}

	template<typename T>
	bool HasComponent()
	{
    return s_registry.any_of<T>(m_entityHandle);
	}

	template<typename T>
	void RemoveComponent()
	{
		MI_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
		s_registry.remove<T>(m_entityHandle);
	}
  
  static Entity CreateEntity(int id, const std::string& name);
  static void DestroyEntity(Entity entity);
  template<typename... Args>
  static auto View()
  {
    return s_registry.view<Args...>();
  }

	operator bool() const { return m_entityHandle != entt::null; }
	operator entt::entity() const { return m_entityHandle; }
	operator uint32_t() const { return (uint32_t)m_entityHandle; }

	int GetUUID() { return GetComponent<IDComponent>().ID; }
	const std::string& GetName() { return GetComponent<TagComponent>().tag; }
  entt::entity GetHandle(){return m_entityHandle;}
	bool operator==(const Entity& other) const
	{
		return m_entityHandle == other.m_entityHandle;
	}

	bool operator!=(const Entity& other) const
	{
		return !(*this == other);
	}
private:
  static entt::registry s_registry;
	entt::entity m_entityHandle{ entt::null };
};
  
} // namespace medicimage
