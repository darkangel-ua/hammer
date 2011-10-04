#include "hammerbuildconfiguration.h"

#include "hammermakestep.h"
#include "hammerproject.h"
#include "hammertarget.h"
#include "hammerprojectconstants.h"

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/buildsteplist.h>
#include <utils/qtcassert.h>

#include <QtGui/QInputDialog>

using ProjectExplorer::BuildConfiguration;

namespace hammer{ namespace QtCreator{

HammerBuildConfiguration::HammerBuildConfiguration(HammerTarget *parent)
    : BuildConfiguration(parent, QLatin1String(HAMMER_BC_ID))
{
}

HammerBuildConfiguration::HammerBuildConfiguration(HammerTarget *parent, HammerBuildConfiguration *source) :
    BuildConfiguration(parent, source)
{
    cloneSteps(source);
}

HammerBuildConfiguration::~HammerBuildConfiguration()
{
}

QString HammerBuildConfiguration::buildDirectory() const
{
   return target()->project()->projectDirectory();
}

HammerTarget *HammerBuildConfiguration::hammerTarget() const
{
    return static_cast<HammerTarget *>(target());
}

ProjectExplorer::IOutputParser *HammerBuildConfiguration::createOutputParser() const
{
    ProjectExplorer::ToolChain *tc = hammerTarget()->hammerProject()->toolChain();
    if (tc)
        return tc->outputParser();
    
    return NULL;
}


HammerBuildConfigurationFactory::HammerBuildConfigurationFactory(QObject *parent) :
    ProjectExplorer::IBuildConfigurationFactory(parent)
{
}

HammerBuildConfigurationFactory::~HammerBuildConfigurationFactory()
{
}

QStringList HammerBuildConfigurationFactory::availableCreationIds(ProjectExplorer::Target *parent) const
{
    if (!qobject_cast<HammerTarget *>(parent))
        return QStringList();

    return QStringList() << QLatin1String(HAMMER_BC_ID);
}

QString HammerBuildConfigurationFactory::displayNameForId(const QString &id) const
{
    if (id == QLatin1String(HAMMER_BC_ID))
        return tr("Build");

    return QString();
}

bool HammerBuildConfigurationFactory::canCreate(ProjectExplorer::Target *parent, const QString &id) const
{
    if (!qobject_cast<HammerTarget *>(parent))
        return false;
    
    if (id == QLatin1String(HAMMER_BC_ID))
        return true;
    
    return false;
}

HammerBuildConfiguration* HammerBuildConfigurationFactory::create(ProjectExplorer::Target *parent, const QString &id)
{
    if (!canCreate(parent, id))
        return 0;
    
    HammerTarget *target(static_cast<HammerTarget *>(parent));

    //TODO asking for name is duplicated everywhere, but maybe more
    // wizards will show up, that incorporate choosing the name
    bool ok;
    QString buildConfigurationName = QInputDialog::getText(0,
                          tr("New Configuration"),
                          tr("New configuration name:"),
                          QLineEdit::Normal,
                          QString(),
                          &ok);
    if (!ok || buildConfigurationName.isEmpty())
        return 0;

    HammerBuildConfiguration *bc = new HammerBuildConfiguration(target);
    // we need to add current file build list, and this is only way for now
    QVariantMap saved_bc = bc->toMap();
    bc->fromMap(saved_bc);

    bc->setDisplayName(buildConfigurationName);

    {
       ProjectExplorer::BuildStepList *buildSteps = bc->stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
       Q_ASSERT(buildSteps);
       HammerMakeStep *makeStep = new HammerMakeStep(buildSteps);
       buildSteps->insertStep(0, makeStep);
       makeStep->setBuildTarget("all", /*on=*/true);

       target->addBuildConfiguration(bc); // also makes the name unique...
    }

    {
       ProjectExplorer::BuildStepList *buildSteps = bc->stepList(HAMMER_BC_BUILD_CURRENT_LIST_ID);
       Q_ASSERT(buildSteps);
       HammerMakeCurrentStep *makeStep = new HammerMakeCurrentStep(buildSteps);
       buildSteps->insertStep(0, makeStep);
       target->addBuildConfiguration(bc); // also makes the name unique...
    }

    return bc;
}

// from QtCreator projectexplorer/buildconfiguration.cpp
static const char* const BUILD_STEP_LIST_COUNT("ProjectExplorer.BuildConfiguration.BuildStepListCount");
static const char* const BUILD_STEP_LIST_PREFIX("ProjectExplorer.BuildConfiguration.BuildStepList.");

QVariantMap HammerBuildConfiguration::toMap() const
{
   QVariantMap result = BuildConfiguration::toMap();

   // we need to add current file build step coz there is no other way to do except toMap/fromMap
   if (stepList(HAMMER_BC_BUILD_CURRENT_LIST_ID) == NULL)
   {
      result[BUILD_STEP_LIST_COUNT] = result[BUILD_STEP_LIST_COUNT].toInt() + 1;
      ProjectExplorer::BuildStepList* bsl = new ProjectExplorer::BuildStepList(const_cast<HammerBuildConfiguration*>(this), HAMMER_BC_BUILD_CURRENT_LIST_ID);
      bsl->insertStep(0, new HammerMakeCurrentStep(bsl));
      result[QString(BUILD_STEP_LIST_PREFIX) + QString::number(result[BUILD_STEP_LIST_COUNT].toInt() - 1)] = bsl->toMap();
      delete bsl;
   }

   return result;
}

bool HammerBuildConfiguration::fromMap(const QVariantMap &map)
{
   return ProjectExplorer::BuildConfiguration::fromMap(map);
}

bool HammerBuildConfigurationFactory::canClone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source) const
{
    return canCreate(parent, source->id());
}

HammerBuildConfiguration* HammerBuildConfigurationFactory::clone(ProjectExplorer::Target *parent, BuildConfiguration *source)
{
    if (!canClone(parent, source))
        return NULL;

    HammerTarget *target(static_cast<HammerTarget *>(parent));
    return new HammerBuildConfiguration(target, qobject_cast<HammerBuildConfiguration *>(source));
}

bool HammerBuildConfigurationFactory::canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const
{
    QString id(ProjectExplorer::idFromMap(map));
    return canCreate(parent, id);
}

HammerBuildConfiguration* HammerBuildConfigurationFactory::restore(ProjectExplorer::Target *parent, const QVariantMap &map)
{
    if (!canRestore(parent, map))
        return NULL;

    HammerTarget* target(static_cast<HammerTarget*>(parent));
    HammerBuildConfiguration* bc(new HammerBuildConfiguration(target));
    if (bc->fromMap(map))
        return bc;

    delete bc;

    return NULL;
}

BuildConfiguration::BuildType HammerBuildConfiguration::buildType() const
{
    return Unknown;
}
}}
