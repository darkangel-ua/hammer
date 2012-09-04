#if !defined(h_a1cc270b_d0ff_4867_ad78_41400c859475)
#define h_a1cc270b_d0ff_4867_ad78_41400c859475

#include <projectexplorer/iprojectmanager.h>
#include <projectexplorer/projectnodes.h>
#include <hammer/core/engine.h>
#include <map>
#include <boost/unordered_set.hpp>

namespace hammer{ class main_target; }
namespace hammer{ namespace QtCreator{

class HammerProject;
class HammerDepProjectNode;

typedef std::map<const ::hammer::main_target*, bool> visible_targets_t;
typedef std::map<std::string /*tag*/, bool> stored_visible_targets_t;

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
      bool hasOpenedProjects() const { return m_projects.size(); }
      const visible_targets_t& get_visible_targets() const { return visible_targets_; }
      void set_visible_targets(const visible_targets_t& v);
      const stored_visible_targets_t& get_stored_visible_targets() const { return stored_visible_targets_; }
      void set_stored_visible_targets(const stored_visible_targets_t& v);
      ProjectExplorer::ProjectNode* add_dep(const main_target& mt, const HammerProject& owner);
      const QList<HammerProject*>& get_projects() const { return m_projects; }

   private slots:
      void on_aboutToLoadSession(QString sessionName);
      void on_sessionLoaded(QString sessionName);

   private:
      typedef std::map<const main_target*, HammerDepProjectNode*> deps_t;

      QList<HammerProject*> m_projects;
      engine m_engine;
      deps_t deps_;
      visible_targets_t visible_targets_;
      stored_visible_targets_t stored_visible_targets_;
      bool show_visible_dialog_on_open_;

      visible_targets_t gather_all_targets(ProjectExplorer::Project* project);
      visible_targets_t gather_all_targets(const main_target& mt);
};

void gatherAllMainTargets(boost::unordered_set<const ::hammer::main_target*>& targets,
                          const ::hammer::main_target& targetToInspect);

}}

#endif //h_a1cc270b_d0ff_4867_ad78_41400c859475
