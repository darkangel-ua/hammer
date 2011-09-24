#if !defined(h_51aa0b54_a286_48db_9fa4_f0b7328e74c2)
#define h_51aa0b54_a286_48db_9fa4_f0b7328e74c2

#include <projectexplorer/project.h>
#include <coreplugin/ifile.h>

namespace ProjectExplorer{ class IProjectManager; }

namespace hammer{ namespace QtCreator{

class ProjectManager;
class HammerProjectFile;
class HammerProjectNode;

class HammerProject : public ProjectExplorer::Project
{
      Q_OBJECT
   public:
      HammerProject(ProjectManager *manager, const QString &fileName);
      virtual ~HammerProject();

      virtual QList<ProjectExplorer::Project*> dependsOn();
      virtual QString displayName() const;
      virtual QString id() const;
      virtual Core::IFile *file() const;
      virtual ProjectExplorer::IProjectManager* projectManager() const;
      virtual ProjectExplorer::ProjectNode *rootProjectNode() const;
      virtual QStringList files(FilesMode fileMode) const;
   
   protected:
      virtual bool fromMap(const QVariantMap &map);
   
   private:
      ProjectManager *m_manager;
      QString m_fileName;
      HammerProjectFile *m_projectFile;
      QString m_projectName;
      HammerProjectNode *m_rootNode;
};

class HammerProjectFile : public Core::IFile
{
    Q_OBJECT

   public:
      HammerProjectFile(HammerProject *parent, QString fileName);
      virtual ~HammerProjectFile();

      virtual bool save(QString *errorString, const QString &fileName, bool autoSave);
      virtual QString fileName() const;

      virtual QString defaultPath() const;
      virtual QString suggestedFileName() const;
      virtual QString mimeType() const;

      virtual bool isModified() const;
      virtual bool isReadOnly() const;
      virtual bool isSaveAsAllowed() const;
      virtual void rename(const QString &newName);

      ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const;
      bool reload(QString *errorString, ReloadFlag flag, ChangeType type);

   private:
      HammerProject *m_project;
      QString m_fileName;
};

}}

#endif //h_51aa0b54_a286_48db_9fa4_f0b7328e74c2
