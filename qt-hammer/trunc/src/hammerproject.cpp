#include <coreplugin/icontext.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <extensionsystem/pluginmanager.h>

#include "hammerprojectmanager.h"
#include "hammerproject.h"
#include "hammerprojectconstants.h"
#include "hammerprojectnode.h"
#include "hammertarget.h"

namespace hammer{ namespace QtCreator{

HammerProject::HammerProject(ProjectManager *manager, 
                             const QString &fileName)
   : m_manager(manager),
     m_fileName(fileName)
{
   setProjectContext(Core::Context(PROJECTCONTEXT));
   setProjectLanguage(Core::Context(ProjectExplorer::Constants::LANG_CXX));
   
   QFileInfo fileInfo(m_fileName);

   m_projectName = fileInfo.completeBaseName();
   m_projectFile = new HammerProjectFile(this, fileName);
   m_rootNode = new HammerProjectNode(this, m_projectFile);

   m_manager->registerProject(this);
}

HammerProject::~HammerProject()
{
   m_manager->unregisterProject(this);

   delete m_rootNode;
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
   return QStringList();
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

   if (targets().isEmpty()) {
      HammerTargetFactory *factory =
         ExtensionSystem::PluginManager::instance()->getObject<HammerTargetFactory>();
      addTarget(factory->create(this, QLatin1String(HAMMER_DESKTOP_TARGET_ID)));
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


}}