#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/customexecutablerunconfiguration.h>
#include <projectexplorer/deployconfiguration.h>
#include <projectexplorer/projectexplorerconstants.h>

#include <QtGui/QApplication>
#include <QtGui/QStyle>
#include <boost/foreach.hpp>

#include <hammer/core/main_target.h>
#include <hammer/core/target_type.h>
#include <hammer/core/types.h>

#include "hammertarget.h"
#include "hammerproject.h"
#include "hammerbuildconfiguration.h"
#include "hammermakestep.h"
#include "hammerprojectconstants.h"

namespace hammer{ namespace QtCreator{

static const char * const HAMMER_DESKTOP_TARGET_DISPLAY_NAME("Desktop");

HammerTarget::HammerTarget(HammerProject *parent) :
    ProjectExplorer::Target(parent, QLatin1String(HAMMER_DESKTOP_TARGET_ID)),
    m_buildConfigurationFactory(new HammerBuildConfigurationFactory(this))
{
    setDefaultDisplayName(QApplication::translate("HammerProjectManager::HammerTarget",
                                                  HAMMER_DESKTOP_TARGET_DISPLAY_NAME));
    setIcon(qApp->style()->standardIcon(QStyle::SP_ComputerIcon));
}

ProjectExplorer::BuildConfigWidget *HammerTarget::createConfigWidget()
{
    return new HammerBuildSettingsWidget(this);
}

HammerProject *HammerTarget::hammerProject() const
{
    return static_cast<HammerProject*>(project());
}

ProjectExplorer::IBuildConfigurationFactory* HammerTarget::buildConfigurationFactory() const
{
    return m_buildConfigurationFactory;
}

ProjectExplorer::BuildConfiguration* HammerTarget::activeBuildConfiguration() const
{
    return Target::activeBuildConfiguration();
}

bool HammerTarget::fromMap(const QVariantMap &map)
{
    if (!Target::fromMap(map))
        return false;

    return true;
}

HammerTargetFactory::HammerTargetFactory(QObject *parent) :
    ITargetFactory(parent)
{
}

bool HammerTargetFactory::supportsTargetId(const QString &id) const
{
    return id == QLatin1String(HAMMER_DESKTOP_TARGET_ID);
}

QStringList HammerTargetFactory::supportedTargetIds(ProjectExplorer::Project *parent) const
{
    if (!qobject_cast<HammerProject*>(parent))
        return QStringList();

    return QStringList() << QLatin1String(HAMMER_DESKTOP_TARGET_ID);
}

QString HammerTargetFactory::displayNameForId(const QString &id) const
{
    if (id == QLatin1String(HAMMER_DESKTOP_TARGET_ID))
        return QCoreApplication::translate("HammerProjectManager::HammerTarget",
                                           HAMMER_DESKTOP_TARGET_DISPLAY_NAME,
                                           "Hammer desktop target display name");
    return QString();
}

bool HammerTargetFactory::canCreate(ProjectExplorer::Project *parent, const QString &id) const
{
    if (!qobject_cast<HammerProject*>(parent))
        return false;

    return id == QLatin1String(HAMMER_DESKTOP_TARGET_ID);
}

HammerTarget *HammerTargetFactory::create(ProjectExplorer::Project *parent, const QString &id)
{
    if (!canCreate(parent, id))
        return NULL;

    HammerProject *hammerProject = static_cast<HammerProject *>(parent);
    HammerTarget *t = new HammerTarget(hammerProject);

    // Set up BuildConfiguration:
    HammerBuildConfiguration *bc = new HammerBuildConfiguration(t);
    // we need to add current file build list, and this is only way for now
    QVariantMap saved_bc = bc->toMap();
    bc->fromMap(saved_bc);

    bc->setDisplayName("debug");

    {
       ProjectExplorer::BuildStepList *buildSteps = bc->stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
       HammerMakeStep *makeStep = new HammerMakeStep(buildSteps);
       buildSteps->insertStep(0, makeStep);
       makeStep->setBuildTarget("debug", /*on=*/true);
       t->addBuildConfiguration(bc);
       t->addRunConfiguration(new HammerRunConfiguration(t));
    }
    {
       ProjectExplorer::BuildStepList *buildSteps = bc->stepList(HAMMER_BC_BUILD_CURRENT_LIST_ID);
       Q_ASSERT(buildSteps);
       HammerMakeCurrentStep *makeStep = new HammerMakeCurrentStep(buildSteps);
       buildSteps->insertStep(0, makeStep);
    }

    return t;
}

bool HammerTargetFactory::canRestore(ProjectExplorer::Project *parent, const QVariantMap &map) const
{
    return canCreate(parent, ProjectExplorer::idFromMap(map));
}

HammerTarget *HammerTargetFactory::restore(ProjectExplorer::Project *parent, const QVariantMap &map)
{
    if (!canRestore(parent, map))
        return 0;
    HammerProject *project = static_cast<HammerProject *>(parent);
    HammerTarget *target = new HammerTarget(project);
    if (target->fromMap(map))
        return target;

    delete target;

    return NULL;
}

HammerRunConfiguration::HammerRunConfiguration(HammerTarget *parent)
   : CustomExecutableRunConfiguration(parent),
     m_target(parent)
{
}

QString HammerRunConfiguration::executable() const
{
   if (m_executable)
      if (m_executable->isEmpty())
         return CustomExecutableRunConfiguration::executable();
      else
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

}}
