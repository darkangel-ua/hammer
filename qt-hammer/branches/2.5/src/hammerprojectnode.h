#if !defined(h_e94fdb51_b369_4d9f_b686_5a6db23a9466)
#define h_e94fdb51_b369_4d9f_b686_5a6db23a9466

#include <projectexplorer/projectnodes.h>

namespace Core
{
   class IFile;
}

namespace hammer{namespace QtCreator{

class HammerProject;

class HammerProjectNode : public ProjectExplorer::ProjectNode
{
   public:
      HammerProjectNode(HammerProject *project, Core::IFile *projectFile);

      Core::IFile *projectFile() const;
      QString projectFilePath() const;

      virtual bool hasBuildTargets() const;

      virtual QList<ProjectExplorer::ProjectNode::ProjectAction> supportedActions(Node *node) const;

      virtual bool canAddSubProject(const QString &proFilePath) const;

      virtual bool addSubProjects(const QStringList &proFilePaths);
      virtual bool removeSubProjects(const QStringList &proFilePaths);

      virtual bool addFiles(const ProjectExplorer::FileType fileType,
                          const QStringList &filePaths,
                          QStringList *notAdded = 0);

      virtual bool removeFiles(const ProjectExplorer::FileType fileType,
                             const QStringList &filePaths,
                             QStringList *notRemoved = 0);
      virtual bool deleteFiles(const ProjectExplorer::FileType fileType,
                             const QStringList &filePaths);

      virtual bool renameFile(const ProjectExplorer::FileType fileType,
                             const QString &filePath,
                             const QString &newFilePath);

      virtual QList<ProjectExplorer::RunConfiguration *> runConfigurationsFor(Node *node);

      void refresh();

   private:
      typedef QHash<QString, FolderNode *> FolderByName;

      HammerProject *m_project;
      Core::IFile *m_projectFile;

      FolderNode* m_srcNode;
      FolderNode* m_incNode;
      FolderNode* m_resNode;
      FolderNode* m_formNode;

      void addNodes(const basic_target* bt);
};

}}

#endif //h_e94fdb51_b369_4d9f_b686_5a6db23a9466
