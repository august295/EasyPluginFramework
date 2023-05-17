#include "Framework.h"

struct Framework::FrameworkPrivate {
	std::shared_ptr<PluginManager> m_PluginManagerSptr;
};

Framework::Framework()
	: m_P(new FrameworkPrivate)
{
	m_P->m_PluginManagerSptr = std::make_shared<PluginManager>();
}

Framework::~Framework()
{
}

std::shared_ptr<PluginManager>& Framework::GetPluginManager()
{
	return m_P->m_PluginManagerSptr;
}
