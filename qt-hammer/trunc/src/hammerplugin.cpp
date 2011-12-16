#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/ifile.h>
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

#include "hammerplugin.h"
#include "hammerprojectmanager.h"
#include "hammerproject.h"
#include "hammerprojectnode.h"
#include "hammereditorfactory.h"
#include "hammerprojectconstants.h"
#include "hammertarget.h"
#include "hammermakestep.h"
#include "hammerrunconfiguration.h"

HammerPlugin::HammerPlugin()
{
}

HammerPlugin::~HammerPlugin()
{
}

bool HammerPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)

    Core::ICore *core = Core::ICore::instance();
    Core::MimeDatabase *mimeDB = core->mimeDatabase();

    const QLatin1String mimetypesXml(":hammerproject/HammerProject.mimetypes.xml");

    if (! mimeDB->addMimeTypes(mimetypesXml, errorMessage))
       return false;

    hammer::QtCreator::ProjectManager *manager = new hammer::QtCreator::ProjectManager();
    addAutoReleasedObject(manager);
    addAutoReleasedObject(new hammer::QtCreator::HammerTargetFactory);
    addAutoReleasedObject(new hammer::QtCreator::HammerMakeStepFactory);
    addAutoReleasedObject(new hammer::QtCreator::HammerRunConfigurationFactory);

    Core::Context context(Core::Constants::C_GLOBAL);

    Core::ActionManager *actionManager = core->actionManager();
    Core::ActionContainer *mproject = actionManager->actionContainer(ProjectExplorer::Constants::M_BUILDPROJECT);
    m_buildCurrentFileAction = new QAction(tr("Build currentFile..."), this);
    m_buildCurrentFileAction->setEnabled(false);
    Core::Command *command = actionManager->registerAction(m_buildCurrentFileAction, "Hammer.BuildCurrentFile", context);
    command->setDefaultKeySequence(QKeySequence(tr("Ctrl+F7")));
    mproject->addAction(command, ProjectExplorer::Constants::G_BUILD_PROJECT);

    connect(m_buildCurrentFileAction, SIGNAL(triggered()), SLOT(buildCurrentFile()));

    connect(core->editorManager(), SIGNAL(currentEditorChanged(Core::IEditor*)), SLOT(currentEditorChanged(Core::IEditor*)));

    return true;
}

void HammerPlugin::buildCurrentFile()
{
   using namespace hammer::QtCreator;

   ProjectExplorer::ProjectExplorerPlugin* projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();
   Core::IEditor* editor = Core::ICore::instance()->editorManager()->currentEditor();
   Core::IFile* file = editor->file();
   if (file)
   {
      ProjectExplorer::Project* p = projectExplorer->session()->projectForFile(file->fileName());
      if (p && p->activeTarget())
      {
         QString err;
         if (file->isModified())
            if (!file->save(&err))
            {
               Core::MessageManager *messageManager = Core::ICore::instance()->messageManager();
               messageManager->printToOutputPanePopup(tr("Failed to save file '%1'")
                                                      .arg(QDir::toNativeSeparators(file->fileName())));
               return;
            }

         ProjectExplorer::Target* target = p->activeTarget();
         ProjectExplorer::BuildStepList* bsl = p->activeTarget()
                                                ->activeBuildConfiguration()
                                                ->stepList(hammer::QtCreator::HAMMER_BC_BUILD_CURRENT_LIST_ID);
         HammerMakeCurrentStep* step = qobject_cast<HammerMakeCurrentStep*>(bsl->at(0));
         QDir projectDir(QFileInfo(p->file()->fileName()).dir());
         step->setTargetToBuid(projectDir.relativeFilePath(file->fileName()), projectDir.absolutePath());
         projectExplorer->buildManager()
                        ->buildList(bsl);
      }
   }
}

void HammerPlugin::currentEditorChanged(Core::IEditor* editor)
{
   if (editor)
   {
      Core::IFile* file = editor->file();
      if (file)
      {
         ProjectExplorer::ProjectExplorerPlugin* projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();
         ProjectExplorer::Node* fileNode = projectExplorer->session()->nodeForFile(file->fileName());
         if (fileNode && dynamic_cast<hammer::QtCreator::HammerProjectNode*>(fileNode->projectNode()))
         {
            m_buildCurrentFileAction->setEnabled(true);
            return;
         }
      }
   }

   m_buildCurrentFileAction->setEnabled(false);
}

void HammerPlugin::extensionsInitialized()
{
}

Q_EXPORT_PLUGIN(HammerPlugin)
