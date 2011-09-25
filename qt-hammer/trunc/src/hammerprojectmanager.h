#if !defined(h_a1cc270b_d0ff_4867_ad78_41400c859475)
#define h_a1cc270b_d0ff_4867_ad78_41400c859475

#include <projectexplorer/iprojectmanager.h>
#include <hammer/core/engine.h>

namespace hammer{namespace QtCreator{

class HammerProject;

class ProjectManager : public ProjectExplorer::IProjectManager
{
      Q_OBJECT

   public:
      ProjectManager();

      virtual QString mimeType() const;
      virtual ProjectExplorer::Project *openProject(const QString& fileName);

      void registerProject(HammerProject* project);
      void unregisterProject(HammerProject* project);
      engine& get_engine() { return m_engine; }

   private:
      QList<HammerProject*> m_projects;
      engine m_engine;
      project* m_hammerMasterProject;
};

}}

#endif //h_a1cc270b_d0ff_4867_ad78_41400c859475
