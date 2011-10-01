#if !defined(h_51aa0b54_a286_48db_9fa4_f0b7328e74c2)
#define h_51aa0b54_a286_48db_9fa4_f0b7328e74c2

#include <projectexplorer/project.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/buildstep.h>
#include <coreplugin/ifile.h>

class QComboBox; 

namespace ProjectExplorer{ class IProjectManager; }
namespace hammer{ class main_target; }
namespace Utils { class PathChooser; }

namespace hammer{ namespace QtCreator{

class ProjectManager;
class HammerProjectFile;
class HammerProjectNode;
class HammerBuildConfiguration;
class HammerTarget;

class HammerProject : public ProjectExplorer::Project
{
      Q_OBJECT

   public:
      HammerProject(ProjectManager *manager, const main_target* mt);
      virtual ~HammerProject();

      virtual QList<ProjectExplorer::Project*> dependsOn();
      virtual QString displayName() const;
      virtual QString id() const;
      virtual Core::IFile *file() const;
      virtual ProjectExplorer::IProjectManager* projectManager() const;
      virtual ProjectExplorer::ProjectNode *rootProjectNode() const;
      virtual QStringList files(FilesMode fileMode) const;

      const main_target& get_main_target() const { return *m_mainTarget; }
      ProjectExplorer::ToolChain *toolChain() const;
      void setToolChain(ProjectExplorer::ToolChain *tc);
   
   signals:
      void toolChainChanged(ProjectExplorer::ToolChain *);

   protected:
      virtual bool fromMap(const QVariantMap &map);
   
   private:
      ProjectManager *m_manager;
      HammerProjectFile *m_projectFile;
      QString m_projectName;
      HammerProjectNode *m_rootNode;
      const main_target* m_mainTarget;
      ProjectExplorer::ToolChain *m_toolChain;
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

class HammerBuildSettingsWidget : public ProjectExplorer::BuildConfigWidget
{
      Q_OBJECT

   public:
      HammerBuildSettingsWidget(HammerTarget *target);
      virtual ~HammerBuildSettingsWidget();

      virtual QString displayName() const;
      virtual void init(ProjectExplorer::BuildConfiguration *bc);

   private Q_SLOTS:
      void toolChainSelected(int index);
      void toolChainChanged(ProjectExplorer::ToolChain *);
      void updateToolChainList();

   private:
      HammerTarget *m_target;
      Utils::PathChooser *m_pathChooser;
      QComboBox *m_toolChainChooser;
      HammerBuildConfiguration *m_buildConfiguration;
};

}}

#endif //h_51aa0b54_a286_48db_9fa4_f0b7328e74c2
