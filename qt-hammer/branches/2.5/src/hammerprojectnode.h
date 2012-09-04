#if !defined(h_e94fdb51_b369_4d9f_b686_5a6db23a9466)
#define h_e94fdb51_b369_4d9f_b686_5a6db23a9466

#include <projectexplorer/projectnodes.h>

namespace Core
{
   class IDocument;
}

namespace hammer{ class main_target; class basic_target; }

namespace hammer{namespace QtCreator{

class HammerProject;
class HammerDepProjectNode;

class HammerNodeBase : public ProjectExplorer::ProjectNode
{
      Q_OBJECT

   protected:
      HammerNodeBase(const QString& projectFilePath);
      void addNodes(const basic_target* bt);
      virtual void refresh();

      FolderNode* m_srcNode;
      FolderNode* m_incNode;
      FolderNode* m_resNode;
      FolderNode* m_formNode;
};

class HammerProjectNode : public HammerNodeBase
{
      Q_OBJECT
   public:
      HammerProjectNode(HammerProject *project, Core::IDocument *projectFile);

      Core::IDocument *projectFile() const;
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

      void wipe_nodes();
      void refresh();
      HammerProject& project() const { return *m_project; }

   private:
      typedef QHash<QString, FolderNode *> FolderByName;

      HammerProject *m_project;
      Core::IDocument *m_projectFile;
};

class HammerDepProjectNode : public HammerNodeBase
{
      Q_OBJECT
   public:
      HammerDepProjectNode(const hammer::main_target& mt, const HammerProject& owner);
      ~HammerDepProjectNode();

      virtual bool hasBuildTargets() const { return false; }

      virtual QList<ProjectExplorer::ProjectNode::ProjectAction> supportedActions(Node *node) const
      { return QList<ProjectExplorer::ProjectNode::ProjectAction>(); }

      virtual bool canAddSubProject(const QString &proFilePath) const { return false; }

      virtual bool addSubProjects(const QStringList &proFilePaths)  { return false; }
      virtual bool removeSubProjects(const QStringList &proFilePaths)  { return false; }

      virtual bool addFiles(const ProjectExplorer::FileType fileType,
                          const QStringList &filePaths,
                          QStringList *notAdded = 0) { return false; }

      virtual bool removeFiles(const ProjectExplorer::FileType fileType,
                             const QStringList &filePaths,
                             QStringList *notRemoved = 0) { return false; }
      virtual bool deleteFiles(const ProjectExplorer::FileType fileType,
                             const QStringList &filePaths) { return false; }

      virtual bool renameFile(const ProjectExplorer::FileType fileType,
                             const QString &filePath,
                             const QString &newFilePath) { return false; }

      virtual QList<ProjectExplorer::RunConfiguration *> runConfigurationsFor(Node *node) { return QList<ProjectExplorer::RunConfiguration *>(); }
      void refresh();
      const HammerProject& owner() const { return owner_; }
      const hammer::main_target& mt() const { return mt_; }

   private:
      const hammer::main_target& mt_;
      const HammerProject& owner_;
};

class HammerDepLinkProjectNode : public HammerNodeBase
{
      Q_OBJECT
   public:
      HammerDepLinkProjectNode(HammerDepProjectNode& link);
      ~HammerDepLinkProjectNode();

      virtual bool hasBuildTargets() const { return false; }

      virtual QList<ProjectExplorer::ProjectNode::ProjectAction> supportedActions(Node *node) const
      { return QList<ProjectExplorer::ProjectNode::ProjectAction>(); }

      virtual bool canAddSubProject(const QString &proFilePath) const { return false; }

      virtual bool addSubProjects(const QStringList &proFilePaths)  { return false; }
      virtual bool removeSubProjects(const QStringList &proFilePaths)  { return false; }

      virtual bool addFiles(const ProjectExplorer::FileType fileType,
                          const QStringList &filePaths,
                          QStringList *notAdded = 0) { return false; }

      virtual bool removeFiles(const ProjectExplorer::FileType fileType,
                             const QStringList &filePaths,
                             QStringList *notRemoved = 0) { return false; }
      virtual bool deleteFiles(const ProjectExplorer::FileType fileType,
                             const QStringList &filePaths) { return false; }

      virtual bool renameFile(const ProjectExplorer::FileType fileType,
                             const QString &filePath,
                             const QString &newFilePath) { return false; }

      virtual QList<ProjectExplorer::RunConfiguration *> runConfigurationsFor(Node *node) { return QList<ProjectExplorer::RunConfiguration *>(); }
      void refresh();
      HammerDepProjectNode& link() { return link_; }

   private:
      HammerDepProjectNode& link_;
};

}}

#endif //h_e94fdb51_b369_4d9f_b686_5a6db23a9466
