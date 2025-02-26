#include "Entity.h"

using namespace ecs;

entity_t::entity_t(entity_id uniqueID)
{
    m_uniqueID = uniqueID;
    m_componentsSignature.reset();
}