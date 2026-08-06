#include "Utility/Collections/ResourceStack.h"

using namespace Fulk;

ResourceStack::~ResourceStack()
{
	while (m_top > -1)
	{
		m_stack[m_top--]();
	}
}