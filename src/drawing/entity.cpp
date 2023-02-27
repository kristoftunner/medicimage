#include "drawing/entity.h"

namespace medicimage {

Entity::Entity(entt::entity handle, DrawingSheet* sheet)
	: m_entityHandle(handle), m_sheet(sheet)
{
}

}
