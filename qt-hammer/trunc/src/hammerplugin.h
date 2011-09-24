#if !defined(h_fe04d5fa_d30a_47e3_8be4_929b56c10690)
#define h_fe04d5fa_d30a_47e3_8be4_929b56c10690

#include <extensionsystem/iplugin.h>

class HammerPlugin : public ExtensionSystem::IPlugin
{
      Q_OBJECT
   
   public:
      HammerPlugin();
      ~HammerPlugin();

      bool initialize(const QStringList &arguments, QString *errorMessage);
      void extensionsInitialized();
};

#endif //h_fe04d5fa_d30a_47e3_8be4_929b56c10690
