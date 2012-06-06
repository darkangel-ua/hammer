#if !defined(h_fe04d5fa_d30a_47e3_8be4_929b56c10690)
#define h_fe04d5fa_d30a_47e3_8be4_929b56c10690

#include <extensionsystem/iplugin.h>

namespace Core{ class IEditor; }
namespace ProjectExplorer { class Project; }

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

   private:
      QAction *m_buildCurrentFileAction;
      ProjectExplorer::Project* m_StartupProject_;
};

#endif //h_fe04d5fa_d30a_47e3_8be4_929b56c10690
