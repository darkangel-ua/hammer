#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <projectexplorer/session.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/ifile.h>
#include <coreplugin/icore.h>
#include <QFileInfo>
#include "hammerprojectmanager.h"
#include "hammerproject.h"
#include "hammerprojectconstants.h"

namespace hammer{namespace QtCreator{

ProjectManager::ProjectManager()
{

}

QString ProjectManager::mimeType() const
{
   return QLatin1String(HAMMERMIMETYPE);
}

ProjectExplorer::Project *ProjectManager::openProject(const QString &fileName)
{
   {
      Core::MessageManager *messageManager = Core::ICore::instance()->messageManager();
      messageManager->printToOutputPanePopup("EBA!!!"); 
   }

   if (!QFileInfo(fileName).isFile())
        return NULL;

    ProjectExplorer::ProjectExplorerPlugin *projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();
    foreach (ProjectExplorer::Project *pi, projectExplorer->session()->projects()) {
        if (fileName == pi->file()->fileName()) {
            Core::MessageManager *messageManager = Core::ICore::instance()->messageManager();
            messageManager->printToOutputPanePopup(tr("Failed opening project '%1': Project already open")
                                                   .arg(QDir::toNativeSeparators(fileName)));
            return NULL;
        }
    }

    HammerProject *project = new HammerProject(this, fileName);
    return project;
}

void ProjectManager::registerProject(HammerProject* project)
{
    m_projects.append(project);
}

void ProjectManager::unregisterProject(HammerProject* project)
{
    m_projects.removeAll(project);
}

void ProjectManager::notifyChanged(const QString& fileName)
{
}

}}