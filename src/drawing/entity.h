#pragma once

#include "drawing/drawing_sheet.h"
#include "drawing/copmonents.h"
#include "core/assert.h"

#include "entt.hpp"

namespace medicimage {

class Entity
{
public:
	Entity() = default;
	Entity(entt::entity handle, DrawingSheet* scene);
	Entity(const Entity& other) = default;

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		MI_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
		T& component = m_sheet->m_registry.emplace<T>(m_entityHandle, std::forward<Args>(args)...);
		m_sheet->OnComponentAdded<T>(*this, component);
		return component;
	}

	template<typename T, typename... Args>
	T& AddOrReplaceComponent(Args&&... args)
	{
		T& component = m_sheet->m_registry.emplace_or_replace<T>(m_entityHandle, std::forward<Args>(args)...);
		m_sheet->OnComponentAdded<T>(*this, component);
		return component;
	}

	template<typename T>
	T& GetComponent()
	{
		MI_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
		return m_sheet->m_registry.get<T>(m_entityHandle);
	}

	template<typename T>
	bool HasComponent()
	{
		return m_sheet->m_registry.has<T>(m_entityHandle);
	}

	template<typename T>
	void RemoveComponent()
	{
		MI_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
		m_sheet->m_registry.remove<T>(m_entityHandle);
	}

	operator bool() const { return m_entityHandle != entt::null; }
	operator entt::entity() const { return m_entityHandle; }
	operator uint32_t() const { return (uint32_t)m_entityHandle; }

	int GetUUID() { return GetComponent<IDComponent>().ID; }
	const std::string& GetName() { return GetComponent<TagComponent>().tag; }

	bool operator==(const Entity& other) const
	{
		return m_entityHandle == other.m_entityHandle && m_sheet == other.m_sheet;
	}

	bool operator!=(const Entity& other) const
	{
		return !(*this == other);
	}
private:
	entt::entity m_entityHandle{ entt::null };
	DrawingSheet* m_sheet = nullptr;
};

}

