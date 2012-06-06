#if !defined(h_a1cc270b_d0ff_4867_ad78_41400c859475)
#define h_a1cc270b_d0ff_4867_ad78_41400c859475

#include <projectexplorer/iprojectmanager.h>
#include <projectexplorer/projectnodes.h>
#include <hammer/core/engine.h>
#include <map>

namespace hammer{ namespace QtCreator{

class HammerProject;
class HammerDepProjectNode;

class ProjectManager : public ProjectExplorer::IProjectManager
{
      Q_OBJECT

   public:
      ProjectManager();

      virtual QString mimeType() const;
      virtual ProjectExplorer::Project *openProject(const QString& fileName, QString *errorString);

      void registerProject(HammerProject* project);
      void unregisterProject(HammerProject* project);
      engine& get_engine() { return m_engine; }

      ProjectExplorer::ProjectNode* add_dep(const main_target& mt, const HammerProject& owner);

   private:
      typedef std::map<const main_target*, HammerDepProjectNode*> deps_t;

      QList<HammerProject*> m_projects;
      engine m_engine;
      deps_t deps_;
};

}}

#endif //h_a1cc270b_d0ff_4867_ad78_41400c859475
