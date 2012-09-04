#if !defined(h_fe04d5fa_d30a_47e3_8be4_929b56c10690)
#define h_fe04d5fa_d30a_47e3_8be4_929b56c10690

#include <extensionsystem/iplugin.h>
#include <string>
#include <map>
#include <utility>

namespace Core{
   class IEditor;
   class ActionContainer;
}

namespace ProjectExplorer { class Project; }
namespace hammer{ namespace QtCreator{ class ProjectManager; }}

class QAction;

class HammerPlugin : public ExtensionSystem::IPlugin
{
      Q_OBJECT

   public:
      HammerPlugin();
      ~HammerPlugin();

      bool initialize(const QStringList &arguments, QString *errorMessage);
      void extensionsInitialized();

   private slots:
      void buildCurrentFile();
      void currentEditorChanged(Core::IEditor* editor);
      void startupProjectChanged(ProjectExplorer::Project* project);
      void projectAdded(ProjectExplorer::Project*);
      void projectRemoved(ProjectExplorer::Project*);
      void on_visibleProjects();
      void on_aboutToLoadSession(QString sessionName);
      void on_sessionLoaded(QString sessionName);

   private:
      QAction *m_buildCurrentFileAction;
      QAction *m_visibleProjectsAction;
      Core::ActionContainer *m_hammerToolsContainer;
      ProjectExplorer::Project* m_StartupProject_;
      hammer::QtCreator::ProjectManager* m_projectManager;

      void save_visible_projects();
};

#endif //h_fe04d5fa_d30a_47e3_8be4_929b56c10690
