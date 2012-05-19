#include <boost/foreach.hpp>

#include <hammer/core/main_target.h>
#include <hammer/core/collect_nodes.h>
#include <hammer/core/target_type.h>
#include <hammer/core/types.h>
#include <hammer/core/engine.h>

#include "hammerrunconfiguration.h"
#include "hammertarget.h"
#include "hammerproject.h"

static const char* const HAMMER_RUN_CONFIGURATION_ID = "HammerPlugin.RunConfiguration";

namespace hammer{ namespace QtCreator{

HammerRunConfiguration::HammerRunConfiguration(HammerTarget *parent)
   : CustomExecutableRunConfiguration(parent),
     m_target(parent)
{
}

HammerRunConfiguration::HammerRunConfiguration(HammerTarget *parent,
                                               HammerRunConfiguration *source)
   : ProjectExplorer::CustomExecutableRunConfiguration(parent, source),
     m_target(parent)
{
}

QString HammerRunConfiguration::executable() const
{
   if (m_executable)
      return *m_executable;

   const main_target& mt = m_target->hammerProject()->get_main_target();
   try
   {
      build_nodes_t nodes = mt.generate();
      BOOST_FOREACH(const build_node_ptr& node, nodes)
         BOOST_FOREACH(const basic_target* bt, node->products_)
            if (bt->type().equal_or_derived_from(types::EXE))
            {
               location_t l = bt->location() / bt->name().to_string();
               m_executable = QString::fromStdString(l.string());
               return *m_executable;
            }

   }catch(...) { }

   return CustomExecutableRunConfiguration::executable();
}

Utils::Environment
HammerRunConfiguration::environment() const
{
   if (!m_additionalPaths)
   {
      const main_target& mt = m_target->hammerProject()->get_main_target();

      try
      {
         typedef std::set<const build_node*> visited_nodes_t;

         build_nodes_t nodes = mt.generate();

         build_node::sources_t collected_nodes;
         visited_nodes_t visited_nodes;
         std::vector<const target_type*> types(1, &mt.get_engine()->get_type_registry().get(types::SHARED_LIB));
         collect_nodes(collected_nodes, visited_nodes, nodes, types, true);

         QStringList paths;
         BOOST_FOREACH(const build_node::source_t& s, collected_nodes)
            paths << QString::fromStdString(s.source_target_->location().string());

         paths.removeDuplicates();
         m_additionalPaths = paths;

      }catch(...) { exit(-1); m_additionalPaths = QStringList(); }
   }

   Utils::Environment env = CustomExecutableRunConfiguration::environment();

   BOOST_FOREACH(const QString& s, *m_additionalPaths)
      env.appendOrSetPath(s);

#if !defined(_WIN32)
   QString sep(":");
   BOOST_FOREACH(const QString& s, *m_additionalPaths)
      env.appendOrSet("LD_LIBRARY_PATH", s, sep);
#endif

   return env;
}

// from projectexplorer/projectconfiguration.cpp
static const char * const CONFIGURATION_ID_KEY("ProjectExplorer.ProjectConfiguration.Id");

QVariantMap HammerRunConfiguration::toMap() const
{
   // there is only one way to make factory works - substitute id
   QVariantMap res = ProjectExplorer::CustomExecutableRunConfiguration::toMap();
   res[CONFIGURATION_ID_KEY] = QLatin1String(HAMMER_RUN_CONFIGURATION_ID);

   return res;
}

HammerRunConfigurationFactory::HammerRunConfigurationFactory(QObject *parent)
   : ProjectExplorer::IRunConfigurationFactory(parent)
{
}

QStringList HammerRunConfigurationFactory::availableCreationIds(ProjectExplorer::Target*) const
{
   return QStringList() << QLatin1String(HAMMER_RUN_CONFIGURATION_ID);
}

QString HammerRunConfigurationFactory::displayNameForId(const QString &id) const
{
   return tr("Hammer Executable");
}

bool HammerRunConfigurationFactory::canCreate(ProjectExplorer::Target *parent,
                                              const QString &id) const
{
   return id == QLatin1String(HAMMER_RUN_CONFIGURATION_ID);
}

ProjectExplorer::RunConfiguration*
HammerRunConfigurationFactory::create(ProjectExplorer::Target *parent,
                                      const QString &id)
{
   if (!canCreate(parent, id))
       return NULL;

   HammerRunConfiguration *rc(new HammerRunConfiguration(static_cast<HammerTarget*>(parent)));
   return rc;
}

bool HammerRunConfigurationFactory::canRestore(ProjectExplorer::Target *parent,
                                               const QVariantMap &map) const
{
   QString id(ProjectExplorer::idFromMap(map));
   return canCreate(parent, id);
}

ProjectExplorer::RunConfiguration*
HammerRunConfigurationFactory::restore(ProjectExplorer::Target *parent,
                                       const QVariantMap &map)
{
   if (!canRestore(parent, map))
       return NULL;

   HammerRunConfiguration *rc(new HammerRunConfiguration(static_cast<HammerTarget*>(parent)));
   if (rc->fromMap(map))
       return rc;

   delete rc;
   return NULL;
}

bool HammerRunConfigurationFactory::canClone(ProjectExplorer::Target *parent,
                                             ProjectExplorer::RunConfiguration *source) const
{
   return canCreate(parent, source->id());
}

ProjectExplorer::RunConfiguration*
HammerRunConfigurationFactory::clone(ProjectExplorer::Target *parent,
                                     ProjectExplorer::RunConfiguration *source)
{
   if (!canClone(parent, source))
       return NULL;

   return new HammerRunConfiguration(static_cast<HammerTarget*>(parent),
                                     static_cast<HammerRunConfiguration*>(source));
}

}}
