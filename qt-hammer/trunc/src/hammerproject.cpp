#include <coreplugin/icontext.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/toolchainmanager.h>
#include <projectexplorer/headerpath.h>
#include <extensionsystem/pluginmanager.h>
#include <cplusplus/ModelManagerInterface.h>

#include <hammer/core/main_target.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/feature.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>

#include "hammerprojectmanager.h"
#include "hammerproject.h"
#include "hammerprojectconstants.h"
#include "hammerprojectnode.h"
#include "hammertarget.h"
#include "hammerbuildconfiguration.h"

#include <QtGui/QFormLayout>
#include <QtGui/QComboBox>
#include <utils/pathchooser.h>
#include <boost/foreach.hpp>

namespace hammer{ namespace QtCreator{

HammerProject::HammerProject(ProjectManager *manager, 
                             const main_target* mt,
                             bool main_project)
   : m_manager(manager),
     m_mainTarget(mt),
     m_toolChain(NULL),
     main_project_(main_project)
{
   setProjectContext(Core::Context(PROJECTCONTEXT));
   setProjectLanguage(Core::Context(ProjectExplorer::Constants::LANG_CXX));
   
   QFileInfo fileInfo(QString::fromStdString((m_mainTarget->location() / "hamfile").native_file_string()));

   m_projectName = QString::fromStdString(mt->name().to_string());
   m_projectFile = new HammerProjectFile(this, fileInfo.absoluteFilePath());
   m_rootNode = new HammerProjectNode(this, m_projectFile);

   m_manager->registerProject(this);
}

HammerProject::~HammerProject()
{
   m_codeModelFuture.cancel();
   m_manager->unregisterProject(this);

   delete m_rootNode;
}

ProjectExplorer::ToolChain *HammerProject::toolChain() const
{
   return m_toolChain;
}

void HammerProject::setToolChain(ProjectExplorer::ToolChain *tc)
{
    if (m_toolChain == tc)
        return;

    m_toolChain = tc;
//    refresh(Configuration);

    foreach (ProjectExplorer::Target *t, targets()) {
        foreach (ProjectExplorer::BuildConfiguration *bc, t->buildConfigurations())
            bc->setToolChain(tc);
    }

    emit toolChainChanged(m_toolChain);
}

QList<ProjectExplorer::Project*> HammerProject::dependsOn()
{
   return QList<ProjectExplorer::Project*>();
}

QString HammerProject::displayName() const
{
   return m_projectName;
}

QString HammerProject::id() const
{
   return QLatin1String(HAMMERPROJECT_ID);
}

Core::IFile* HammerProject::file() const
{
   return m_projectFile;
}

ProjectExplorer::IProjectManager* HammerProject::projectManager() const
{
   return m_manager;
}

ProjectExplorer::ProjectNode* HammerProject::rootProjectNode() const
{
   return m_rootNode;
}

QStringList HammerProject::files(FilesMode fileMode) const
{
   if (!m_files.isEmpty())
      return m_files;

   m_files = QStringList();

   BOOST_FOREACH(const basic_target* bt, get_main_target().sources())
   {
      if (bt->type().equal_or_derived_from(types::CPP) || 
          bt->type().equal_or_derived_from(types::C))
      {
         m_files.append(QString::fromStdString(bt->full_path().string()));
      }
   }

   return m_files;
}

void HammerProject::refresh()
{
   CPlusPlus::CppModelManagerInterface *modelManager =
      CPlusPlus::CppModelManagerInterface::instance();

   if (modelManager) {
      CPlusPlus::CppModelManagerInterface::ProjectInfo pinfo = modelManager->projectInfo(this);

      if (m_toolChain) {
         pinfo.defines = m_toolChain->predefinedMacros();
         pinfo.defines += '\n';

         foreach (const ProjectExplorer::HeaderPath &headerPath, m_toolChain->systemHeaderPaths()) {
            if (headerPath.kind() == ProjectExplorer::HeaderPath::FrameworkHeaderPath)
               pinfo.frameworkPaths.append(headerPath.path());
            else
               pinfo.includePaths.append(headerPath.path());
         }
      }

      pinfo.includePaths += allIncludePaths();
      pinfo.defines += allDefines().join("\n");

      // ### add _defines.
      pinfo.sourceFiles = files(AllFiles);
//      pinfo.sourceFiles += generated();

      QStringList filesToUpdate;
      filesToUpdate = pinfo.sourceFiles;
      m_codeModelFuture.cancel();

      modelManager->updateProjectInfo(pinfo);
      m_codeModelFuture = modelManager->updateSourceFiles(filesToUpdate);
   }
}

QStringList HammerProject::allIncludePaths() const
{
   QStringList result;
   
   BOOST_FOREACH(const feature* f, m_mainTarget->properties())
   {
      if (f->name() == "include")
      {
         location_t l = f->get_path_data().target_->location() / f->value().to_string();
         l.normalize();
         result.append(QString::fromStdString(l.native_file_string()));
      }
   }

   return result;
}

QStringList HammerProject::allDefines() const
{
   QStringList result;
   BOOST_FOREACH(const feature* f, m_mainTarget->properties())
   {
      if (f->name() == "define")
      {
         QString v(f->value().to_string().c_str());
         v.replace(QString("="), QString(" "));
         result.append("#define " + v);
      }
   }

   return result;
}

HammerProjectFile::HammerProjectFile(HammerProject *parent, QString fileName)
   : Core::IFile(parent),
     m_project(parent),
     m_fileName(fileName)
{
}

bool HammerProject::fromMap(const QVariantMap &map)
{
   if (!Project::fromMap(map))
      return false;

   if (targets().isEmpty())
   {
      HammerTargetFactory *factory =
         ExtensionSystem::PluginManager::instance()->getObject<HammerTargetFactory>();
      addTarget(factory->create(this, QLatin1String(HAMMER_DESKTOP_TARGET_ID)));
   }

   // setup toolchain
   QList<ProjectExplorer::ToolChain*> tcs = ProjectExplorer::ToolChainManager::instance()->toolChains();
   foreach (ProjectExplorer::ToolChain *tc, tcs)
      if (tc->typeName() == "MSVC")
      {
         m_toolChain = tc;
         break;
      }

   return true;
}

HammerProjectFile::~HammerProjectFile()
{

}

bool HammerProjectFile::save(QString*, const QString&, bool)
{
    return false;
}

QString HammerProjectFile::fileName() const
{
    return m_fileName;
}

QString HammerProjectFile::defaultPath() const
{
    return QString();
}

QString HammerProjectFile::suggestedFileName() const
{
    return QString();
}

QString HammerProjectFile::mimeType() const
{
    return HAMMERMIMETYPE;
}

bool HammerProjectFile::isModified() const
{
    return false;
}

bool HammerProjectFile::isReadOnly() const
{
    return true;
}

bool HammerProjectFile::isSaveAsAllowed() const
{
    return false;
}

void HammerProjectFile::rename(const QString&)
{
}

Core::IFile::ReloadBehavior HammerProjectFile::reloadBehavior(ChangeTrigger, ChangeType) const
{
    return BehaviorSilent;
}

bool HammerProjectFile::reload(QString*, ReloadFlag, ChangeType)
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////////
// HammerBuildSettingsWidget
////////////////////////////////////////////////////////////////////////////////////

HammerBuildSettingsWidget::HammerBuildSettingsWidget(HammerTarget *target)
    : m_target(target), m_toolChainChooser(0), m_buildConfiguration(0)
{
    QFormLayout *fl = new QFormLayout(this);
    fl->setContentsMargins(0, -1, 0, -1);
    fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    // build directory
    m_pathChooser = new Utils::PathChooser(this);
    m_pathChooser->setEnabled(true);
    m_pathChooser->setBaseDirectory(m_target->hammerProject()->projectDirectory());
    fl->addRow(tr("Build directory:"), m_pathChooser);
//    connect(m_pathChooser, SIGNAL(changed(QString)), this, SLOT(buildDirectoryChanged()));

    // tool chain
    m_toolChainChooser = new QComboBox;
    m_toolChainChooser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    updateToolChainList();

    fl->addRow(tr("Tool chain:"), m_toolChainChooser);
    connect(m_toolChainChooser, SIGNAL(activated(int)), this, SLOT(toolChainSelected(int)));
    connect(m_target->hammerProject(), SIGNAL(toolChainChanged(ProjectExplorer::ToolChain*)),
            this, SLOT(toolChainChanged(ProjectExplorer::ToolChain*)));
    connect(ProjectExplorer::ToolChainManager::instance(), SIGNAL(toolChainAdded(ProjectExplorer::ToolChain*)),
            this, SLOT(updateToolChainList()));
    connect(ProjectExplorer::ToolChainManager::instance(), SIGNAL(toolChainRemoved(ProjectExplorer::ToolChain*)),
            this, SLOT(updateToolChainList()));
}

HammerBuildSettingsWidget::~HammerBuildSettingsWidget()
{ }

QString HammerBuildSettingsWidget::displayName() const
{ return tr("Hammer Manager"); }

void HammerBuildSettingsWidget::init(ProjectExplorer::BuildConfiguration *bc)
{
    m_buildConfiguration = static_cast<HammerBuildConfiguration*>(bc);
//    m_pathChooser->setPath(m_buildConfiguration->rawBuildDirectory());
}

void HammerBuildSettingsWidget::toolChainSelected(int index)
{
    using namespace ProjectExplorer;

    ToolChain *tc = static_cast<ToolChain *>(m_toolChainChooser->itemData(index).value<void *>());
    m_target->hammerProject()->setToolChain(tc);
}

void HammerBuildSettingsWidget::toolChainChanged(ProjectExplorer::ToolChain *tc)
{
    for (int i = 0; i < m_toolChainChooser->count(); ++i) {
        ProjectExplorer::ToolChain * currentTc = static_cast<ProjectExplorer::ToolChain *>(m_toolChainChooser->itemData(i).value<void *>());
        if (currentTc != tc)
            continue;

        m_toolChainChooser->setCurrentIndex(i);

        return;
    }
}

void HammerBuildSettingsWidget::updateToolChainList()
{
    m_toolChainChooser->clear();

    QList<ProjectExplorer::ToolChain *> tcs = ProjectExplorer::ToolChainManager::instance()->toolChains();
    if (!m_target->hammerProject()->toolChain()) {
        m_toolChainChooser->addItem(tr("<Invalid tool chain>"), qVariantFromValue(static_cast<void *>(0)));
        m_toolChainChooser->setCurrentIndex(0);
    }

    foreach (ProjectExplorer::ToolChain *tc, tcs) {
        m_toolChainChooser->addItem(tc->displayName(), qVariantFromValue(static_cast<void *>(tc)));
        if (m_target->hammerProject()->toolChain() && 
            m_target->hammerProject()->toolChain()->id() == tc->id()) 
        {
            m_toolChainChooser->setCurrentIndex(m_toolChainChooser->count() - 1);
        }
    }
}


}}
