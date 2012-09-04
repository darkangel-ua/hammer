#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/idocument.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/messagemanager.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/session.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/buildmanager.h>
#include <projectexplorer/buildconfiguration.h>

#include <QStringList>
#include <QtPlugin>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/map.hpp>
#include <hammer/core/main_target.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/toolsets/qt_toolset.h>

#include "hammerplugin.h"
#include "hammerprojectmanager.h"
#include "hammerproject.h"
#include "hammerprojectnode.h"
#include "hammereditorfactory.h"
#include "hammerprojectconstants.h"
#include "hammertarget.h"
#include "hammermakestep.h"
#include "hammerrunconfiguration.h"
#include "visibleprojectsdialog.h"

using namespace hammer;
using namespace hammer::QtCreator;
using namespace std;

HammerPlugin::HammerPlugin() : m_StartupProject_(NULL)
{}

HammerPlugin::~HammerPlugin()
{}

bool HammerPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)

    Core::ICore *core = Core::ICore::instance();
    Core::MimeDatabase *mimeDB = core->mimeDatabase();

    const QLatin1String mimetypesXml(":hammerproject/HammerProject.mimetypes.xml");

    if (! mimeDB->addMimeTypes(mimetypesXml, errorMessage))
       return false;

    m_projectManager = new hammer::QtCreator::ProjectManager();
    addAutoReleasedObject(m_projectManager);
    addAutoReleasedObject(new HammerTargetFactory);
    addAutoReleasedObject(new HammerMakeStepFactory);
    addAutoReleasedObject(new HammerRunConfigurationFactory);

    Core::Context context(Core::Constants::C_GLOBAL);

    Core::ActionManager *actionManager = core->actionManager();

    Core::ActionContainer *mproject = actionManager->actionContainer(ProjectExplorer::Constants::M_BUILDPROJECT);
    m_buildCurrentFileAction = new QAction(tr("Build currentFile"), this);
    m_buildCurrentFileAction->setEnabled(false);
    Core::Command *command = actionManager->registerAction(m_buildCurrentFileAction, "Hammer.BuildCurrentFile", context);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+F7")));
    mproject->addAction(command, ProjectExplorer::Constants::G_BUILD_PROJECT);
    connect(m_buildCurrentFileAction, SIGNAL(triggered()), SLOT(buildCurrentFile()));

    m_hammerToolsContainer = actionManager->createMenu(Core::Id("Hammer.HammerMenu"));
    QMenu *menu = m_hammerToolsContainer->menu();
    menu->setTitle(tr("Hammer"));

    m_visibleProjectsAction = new QAction(tr("Visible projects..."), this);
    m_visibleProjectsAction->setEnabled(false);
    connect(m_visibleProjectsAction, SIGNAL(triggered()), SLOT(on_visibleProjects()));
    Core::Command *visibleProjectsCommand = actionManager->registerAction(m_visibleProjectsAction, "Hammer.VisibleProjects", context);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+6")));
    m_hammerToolsContainer->addAction(visibleProjectsCommand);

    Core::ActionContainer *toolsMenu = actionManager->actionContainer(Core::Id(Core::Constants::M_TOOLS));
    toolsMenu->addMenu(m_hammerToolsContainer);

    connect(core->editorManager(), SIGNAL(currentEditorChanged(Core::IEditor*)), SLOT(currentEditorChanged(Core::IEditor*)));
    ProjectExplorer::ProjectExplorerPlugin* projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();
    connect(projectExplorer->session(),
            SIGNAL(startupProjectChanged(ProjectExplorer::Project*)),
            SLOT(startupProjectChanged(ProjectExplorer::Project*)));

    connect(projectExplorer->session(),
            SIGNAL(projectAdded(ProjectExplorer::Project*)),
            SLOT(projectAdded(ProjectExplorer::Project*)));

    connect(projectExplorer->session(),
            SIGNAL(projectRemoved(ProjectExplorer::Project*)),
            SLOT(projectRemoved(ProjectExplorer::Project*)));

    connect(projectExplorer->session(),
            SIGNAL(aboutToLoadSession(QString)),
            SLOT(on_aboutToLoadSession(QString)));

    connect(projectExplorer->session(),
            SIGNAL(sessionLoaded(QString)),
            SLOT(on_sessionLoaded(QString)));

    return true;
}

static const hammer::main_target* find_mt(const ProjectExplorer::Node& node)
{
   const ProjectExplorer::FolderNode* parent = node.parentFolderNode();
   if (parent == NULL)
      return NULL;

   if (const HammerDepProjectNode* pn = dynamic_cast<const HammerDepProjectNode*>(parent))
      return &pn->mt();
   else if (const HammerProjectNode* pn = dynamic_cast<const HammerProjectNode*>(parent))
      return &pn->project().get_main_target();
   else
      return find_mt(*parent);
}

void HammerPlugin::buildCurrentFile()
{
   using namespace hammer::QtCreator;

   ProjectExplorer::ProjectExplorerPlugin* projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();
   Core::IEditor* editor = Core::ICore::instance()->editorManager()->currentEditor();
   if (Core::IDocument* file = editor->document()) {
      if (ProjectExplorer::Node* file_node = projectExplorer->session()->nodeForFile(file->fileName())) {
         if (const main_target* mt = find_mt(*file_node)) {
            ProjectExplorer::Project* p = m_StartupProject_;
            if (p && p->activeTarget()) {
               QString err;
               if (file->isModified())
                  if (!file->save(&err)) {
                     Core::MessageManager *messageManager = Core::ICore::instance()->messageManager();
                     messageManager->printToOutputPanePopup(tr("Failed to save file '%1'")
                                                            .arg(QDir::toNativeSeparators(file->fileName())));
                     return;
                  }

               ProjectExplorer::BuildStepList* bsl = p->activeTarget()
                                                      ->activeBuildConfiguration()
                                                      ->stepList(hammer::QtCreator::HAMMER_BC_BUILD_CURRENT_LIST_ID);
               HammerMakeCurrentStep* step = qobject_cast<HammerMakeCurrentStep*>(bsl->at(0));
               QDir projectDir(QString::fromStdString(mt->location().string()));
               step->setTargetToBuid(projectDir.relativeFilePath(file->fileName()), projectDir.absolutePath());
               projectExplorer->buildManager()->buildList(bsl, QString());
            }
         }
      }
   }
}

void HammerPlugin::currentEditorChanged(Core::IEditor* editor)
{
   if (editor) {
      Core::IDocument* file = editor->document();
      if (file) {
         ProjectExplorer::ProjectExplorerPlugin* projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();
         ProjectExplorer::Node* fileNode = projectExplorer->session()->nodeForFile(file->fileName());
         if (fileNode && dynamic_cast<hammer::QtCreator::HammerNodeBase*>(fileNode->projectNode())) {
            m_buildCurrentFileAction->setEnabled(true);
            return;
         }
      }
   }

   m_buildCurrentFileAction->setEnabled(false);
}

void HammerPlugin::startupProjectChanged(ProjectExplorer::Project* project)
{
   m_StartupProject_ = project;
}

void HammerPlugin::projectAdded(ProjectExplorer::Project* p)
{
   m_visibleProjectsAction->setEnabled(m_projectManager->hasOpenedProjects());
   save_visible_projects();
}

void HammerPlugin::projectRemoved(ProjectExplorer::Project*)
{
   m_visibleProjectsAction->setEnabled(m_projectManager->hasOpenedProjects());
   save_visible_projects();
}

void HammerPlugin::save_visible_projects()
{
   stored_visible_targets_t svts;
   for(const visible_targets_t::value_type& v : m_projectManager->get_visible_targets())
      svts.insert(make_pair(v.first->location().string(), v.second));

   stringstream s; {
      boost::archive::text_oarchive ar(s);
      ar << svts;
   }

   ProjectExplorer::ProjectExplorerPlugin::instance()->session()->setValue("HammerVisibleTargets", QString::fromStdString(s.str()));
}

void HammerPlugin::on_visibleProjects()
{
   VisibleProjectsDialog dlg(m_projectManager->get_visible_targets());
   if (dlg.exec() == QDialog::Accepted) {
      m_projectManager->set_visible_targets(dlg.targets_);
      save_visible_projects();
   }
}

void HammerPlugin::on_aboutToLoadSession(QString sessionName)
{
   QVariant v = ProjectExplorer::ProjectExplorerPlugin::instance()->session()->value("HammerVisibleTargets");
   if (v.isValid())
      try {
         stringstream s(v.toString().toStdString()); {
            stored_visible_targets_t svts;
            boost::archive::text_iarchive ar(s);
            ar >> svts;
            m_projectManager->set_stored_visible_targets(svts);
         }
      } catch(const std::exception& e) {

      }
}

void HammerPlugin::on_sessionLoaded(QString sessionName)
{
   m_projectManager->set_stored_visible_targets(stored_visible_targets_t());
}

void HammerPlugin::extensionsInitialized()
{
}

Q_EXPORT_PLUGIN(HammerPlugin)
