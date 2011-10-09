#include "hammermakestep.h"
#include "hammerprojectconstants.h"
#include "hammerproject.h"
#include "hammertarget.h"
#include "hammerbuildconfiguration.h"

#include <extensionsystem/pluginmanager.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/gnumakeparser.h>
#include <coreplugin/variablemanager.h>
#include <utils/stringutils.h>
#include <utils/qtcassert.h>
#include <utils/qtcprocess.h>
#include <QCoreApplication>
#include <QListWidgetItem>

namespace {
   const char * const HAMMER_MS_ID("HammerProjectManager.HammerMakeStep");
   const char * const HAMMER_MS_DISPLAY_NAME(QT_TRANSLATE_NOOP("HammerProjectManager::Internal::HammerMakeStep", "Make"));
   const char * const HAMMER_MAKE_CURRENT_DISPLAY_NAME(QT_TRANSLATE_NOOP("HammerProjectManager::Internal::HammerMakeCurrentStep", "MakeCurrent"));

   const char * const BUILD_TARGETS_KEY("HammerProjectManager.HammerMakeStep.BuildTargets");
   const char * const MAKE_ARGUMENTS_KEY("HammerProjectManager.HammerMakeStep.MakeArguments");
   const char * const MAKE_COMMAND_KEY("HammerProjectManager.HammerMakeStep.MakeCommand");
}

namespace hammer{ namespace QtCreator{

HammerMakeStep::HammerMakeStep(ProjectExplorer::BuildStepList *parent) :
AbstractProcessStep(parent, QLatin1String(HAMMER_MS_ID))
{
   ctor();
}

HammerMakeStep::HammerMakeStep(ProjectExplorer::BuildStepList *parent, const QString &id) :
AbstractProcessStep(parent, id)
{
   ctor();
}

HammerMakeStep::HammerMakeStep(ProjectExplorer::BuildStepList *parent, HammerMakeStep *bs) 
   : AbstractProcessStep(parent, bs),
     m_buildTargets(bs->m_buildTargets),
     m_makeArguments(bs->m_makeArguments),
     m_makeCommand(bs->m_makeCommand)
{
   ctor();
}

void HammerMakeStep::ctor()
{
   setDefaultDisplayName(QCoreApplication::translate("HammerProjectManager::Internal::HammerMakeStep", HAMMER_MS_DISPLAY_NAME));
   m_makeCommand = "dhammer";
}

HammerMakeStep::~HammerMakeStep()
{
}

HammerBuildConfiguration *HammerMakeStep::hammerBuildConfiguration() const
{
   return static_cast<HammerBuildConfiguration *>(buildConfiguration());
}

bool HammerMakeStep::init()
{
   HammerBuildConfiguration *bc = hammerBuildConfiguration();

   setEnabled(true);
   ProjectExplorer::ProcessParameters *pp = processParameters();
   pp->setMacroExpander(bc->macroExpander());
   pp->setWorkingDirectory(bc->buildDirectory());
   pp->setEnvironment(bc->environment());
   pp->setCommand(makeCommand());
   pp->setArguments(allArguments());

   setOutputParser(new ProjectExplorer::GnuMakeParser());
   if (bc->hammerTarget()->hammerProject()->toolChain())
      appendOutputParser(bc->hammerTarget()->hammerProject()->toolChain()->outputParser());
   outputParser()->setWorkingDirectory(pp->effectiveWorkingDirectory());

   return AbstractProcessStep::init();
}

QVariantMap HammerMakeStep::toMap() const
{
   QVariantMap map(AbstractProcessStep::toMap());

   map.insert(QLatin1String(BUILD_TARGETS_KEY), m_buildTargets);
   map.insert(QLatin1String(MAKE_ARGUMENTS_KEY), m_makeArguments);
   map.insert(QLatin1String(MAKE_COMMAND_KEY), m_makeCommand);
   return map;
}

bool HammerMakeStep::fromMap(const QVariantMap &map)
{
   m_buildTargets = map.value(QLatin1String(BUILD_TARGETS_KEY)).toStringList();
   m_makeArguments = map.value(QLatin1String(MAKE_ARGUMENTS_KEY)).toString();
   m_makeCommand = map.value(QLatin1String(MAKE_COMMAND_KEY)).toString();

   return BuildStep::fromMap(map);
}

QString HammerMakeStep::allArguments() const
{
   QString args = m_makeArguments;
   Utils::QtcProcess::addArgs(&args, m_buildTargets);
   return args;
}

QString HammerMakeStep::makeCommand() const
{
   return m_makeCommand;
}

void HammerMakeStep::run(QFutureInterface<bool> &fi)
{
   AbstractProcessStep::run(fi);
}

ProjectExplorer::BuildStepConfigWidget *HammerMakeStep::createConfigWidget()
{
   return new HammerMakeStepConfigWidget(this);
}

bool HammerMakeStep::immutable() const
{
   return false;
}

bool HammerMakeStep::buildsTarget(const QString &target) const
{
   return m_buildTargets.contains(target);
}

void HammerMakeStep::setBuildTarget(const QString &target, bool on)
{
   QStringList old = m_buildTargets;
   if (on && !old.contains(target))
      old << target;
   else if(!on && old.contains(target))
      old.removeOne(target);

   m_buildTargets = old;
}

HammerMakeCurrentStep::HammerMakeCurrentStep(ProjectExplorer::BuildStepList *parent)
: AbstractProcessStep(parent, QLatin1String(HAMMER_MAKE_CURRENT_ID))
{

}

HammerMakeCurrentStep::HammerMakeCurrentStep(ProjectExplorer::BuildStepList *parent, HammerMakeCurrentStep *bs)
   : AbstractProcessStep(parent, bs)
{

}

ProjectExplorer::BuildStepConfigWidget* 
HammerMakeCurrentStep::createConfigWidget()
{
   return NULL;
}

bool HammerMakeCurrentStep::init()
{
   HammerBuildConfiguration *bc = hammerBuildConfiguration();

   setEnabled(true);
   ProjectExplorer::ProcessParameters *pp = processParameters();
   pp->setMacroExpander(bc->macroExpander());
   pp->setWorkingDirectory(bc->buildDirectory());
   pp->setEnvironment(bc->environment());
   pp->setCommand("dhammer");

   setOutputParser(new ProjectExplorer::GnuMakeParser());
   if (bc->hammerTarget()->hammerProject()->toolChain())
      appendOutputParser(bc->hammerTarget()->hammerProject()->toolChain()->outputParser());
   outputParser()->setWorkingDirectory(pp->effectiveWorkingDirectory());

   return AbstractProcessStep::init();
}

void HammerMakeCurrentStep::setTargetToBuid(const QString& target, const QString& projectPath)
{
   ProjectExplorer::ProcessParameters *pp = processParameters();
   pp->setArguments("--just-one-source \"" + target + "\" --just-one-source-project-path \"" + projectPath + "\"");
}

HammerBuildConfiguration *HammerMakeCurrentStep::hammerBuildConfiguration() const
{
   return static_cast<HammerBuildConfiguration *>(buildConfiguration());
}

HammerMakeStepFactory::HammerMakeStepFactory(QObject *parent) 
   : ProjectExplorer::IBuildStepFactory(parent)
{
}

HammerMakeStepFactory::~HammerMakeStepFactory()
{
}

bool HammerMakeStepFactory::canCreate(ProjectExplorer::BuildStepList *parent,
                                      const QString &id) const
{
   if (parent->target()->project()->id() != QLatin1String(HAMMERPROJECT_ID))
      return false;

   return id == QLatin1String(HAMMER_MS_ID) ||
          id == QLatin1String(HAMMER_MAKE_CURRENT_ID);
}

ProjectExplorer::BuildStep *HammerMakeStepFactory::create(ProjectExplorer::BuildStepList *parent,
                                                          const QString &id)
{
   if (!canCreate(parent, id))
      return NULL;

   if (id == HAMMER_MAKE_CURRENT_ID)
      return new HammerMakeCurrentStep(parent);
   else
      return new HammerMakeStep(parent);
}

bool HammerMakeStepFactory::canClone(ProjectExplorer::BuildStepList *parent,
                                     ProjectExplorer::BuildStep *source) const
{
   const QString id(source->id());
   return canCreate(parent, id);
}

ProjectExplorer::BuildStep *HammerMakeStepFactory::clone(ProjectExplorer::BuildStepList *parent,
                                                         ProjectExplorer::BuildStep *source)
{
   if (!canClone(parent, source))
      return NULL;

   if (HammerMakeStep* old = qobject_cast<HammerMakeStep*>(source))
      return new HammerMakeStep(parent, old);
   else
      if (HammerMakeCurrentStep* old = qobject_cast<HammerMakeCurrentStep*>(source))
         return new HammerMakeCurrentStep(parent, old);
      else
         return NULL;
}

bool HammerMakeStepFactory::canRestore(ProjectExplorer::BuildStepList *parent,
                                        const QVariantMap &map) const
{
   QString id(ProjectExplorer::idFromMap(map));
   return canCreate(parent, id);
}

ProjectExplorer::BuildStep *HammerMakeStepFactory::restore(ProjectExplorer::BuildStepList *parent,
                                                           const QVariantMap &map)
{
   if (!canRestore(parent, map))
      return NULL;
   
   ProjectExplorer::BuildStep* bs = create(parent, ProjectExplorer::idFromMap(map));
   if (!bs)
      return NULL;

   if (bs->fromMap(map))
      return bs;

   delete bs;
   return NULL;
}

QStringList HammerMakeStepFactory::availableCreationIds(ProjectExplorer::BuildStepList *parent) const
{
   if (parent->target()->project()->id() != QLatin1String(HAMMERPROJECT_ID))
      return QStringList();

   return QStringList() << QLatin1String(HAMMER_MS_ID) << QLatin1String(HAMMER_MAKE_CURRENT_ID);
}

QString HammerMakeStepFactory::displayNameForId(const QString &id) const
{
   if (id == QLatin1String(HAMMER_MS_ID))
      return QCoreApplication::translate("HammerProjectManager::Internal::HammerMakeStep", HAMMER_MS_DISPLAY_NAME);

   return QString();
}

//
// HammerMakeStepConfigWidget
//

HammerMakeStepConfigWidget::HammerMakeStepConfigWidget(HammerMakeStep *makeStep)
    : m_makeStep(makeStep)
{
/*
    m_ui = new Ui::HammerMakeStep;
    m_ui->setupUi(this);

    // TODO update this list also on rescans of the HammerLists.txt
    HammerProject *pro = m_makeStep->genericBuildConfiguration()->genericTarget()->genericProject();
    foreach (const QString &target, pro->buildTargets()) {
        QListWidgetItem *item = new QListWidgetItem(target, m_ui->targetsList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(m_makeStep->buildsTarget(item->text()) ? Qt::Checked : Qt::Unchecked);
    }

    m_ui->makeLineEdit->setText(m_makeStep->m_makeCommand);
    m_ui->makeArgumentsLineEdit->setText(m_makeStep->m_makeArguments);
    updateMakeOverrrideLabel();
    updateDetails();

    connect(m_ui->targetsList, SIGNAL(itemChanged(QListWidgetItem*)),
            this, SLOT(itemChanged(QListWidgetItem*)));
    connect(m_ui->makeLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(makeLineEditTextEdited()));
    connect(m_ui->makeArgumentsLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(makeArgumentsLineEditTextEdited()));

    connect(ProjectExplorer::ProjectExplorerPlugin::instance(), SIGNAL(settingsChanged()),
            this, SLOT(updateMakeOverrrideLabel()));
    connect(ProjectExplorer::ProjectExplorerPlugin::instance(), SIGNAL(settingsChanged()),
            this, SLOT(updateDetails()));
*/
}

QString HammerMakeStepConfigWidget::displayName() const
{
    return tr("Hammer", "HammerMakestep display name.");
}

// TODO: Label should update when tool chain is changed
void HammerMakeStepConfigWidget::updateMakeOverrrideLabel()
{
//    m_ui->makeLabel->setText(tr("Override %1:").arg(m_makeStep->makeCommand()));
}

void HammerMakeStepConfigWidget::updateDetails()
{
    HammerBuildConfiguration *bc = m_makeStep->hammerBuildConfiguration();

    ProjectExplorer::ProcessParameters param;
    param.setMacroExpander(bc->macroExpander());
    param.setWorkingDirectory(bc->buildDirectory());
    param.setEnvironment(bc->environment());
    param.setCommand(m_makeStep->makeCommand());
    param.setArguments(m_makeStep->allArguments());
    m_summaryText = param.summary(displayName());
    emit updateSummary();
}

QString HammerMakeStepConfigWidget::summaryText() const
{
    return m_summaryText;
}

void HammerMakeStepConfigWidget::itemChanged(QListWidgetItem *item)
{
    m_makeStep->setBuildTarget(item->text(), item->checkState() & Qt::Checked);
    updateDetails();
}

void HammerMakeStepConfigWidget::makeLineEditTextEdited()
{
//    m_makeStep->m_makeCommand = m_ui->makeLineEdit->text();
    updateDetails();
}

void HammerMakeStepConfigWidget::makeArgumentsLineEditTextEdited()
{
//    m_makeStep->m_makeArguments = m_ui->makeArgumentsLineEdit->text();
    updateDetails();
}

}}
