#if !defined(h_a1cc270b_d0ff_4867_ad78_41400c859475)
#define h_a1cc270b_d0ff_4867_ad78_41400c859475

#include <projectexplorer/iprojectmanager.h>

namespace hammer{namespace QtCreator{

class HammerProject;

class ProjectManager : public ProjectExplorer::IProjectManager
{
      Q_OBJECT

   public:
      ProjectManager();

      virtual QString mimeType() const;
      virtual ProjectExplorer::Project *openProject(const QString& fileName);

      void notifyChanged(const QString& fileName);

      void registerProject(HammerProject* project);
      void unregisterProject(HammerProject* project);

private:
   QList<HammerProject*> m_projects;
};

}}

#endif //h_a1cc270b_d0ff_4867_ad78_41400c859475
