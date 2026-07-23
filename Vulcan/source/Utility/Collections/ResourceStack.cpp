#include "Utility/Collections/ResourceStack.h"

ResourceStack::~ResourceStack()
{
	while (m_top > -1)
	{
		m_stack[m_top--]();
	}
}