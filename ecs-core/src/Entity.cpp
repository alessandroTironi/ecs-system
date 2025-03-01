#include "Entity.h"

using namespace ecs;

entity_t::entity_t()
{
    m_componentsSignature.reset();
}