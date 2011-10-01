#if !defined(h_7a123cb7_7fc3_4180_ae0c_6c4d3c626198)
#define h_7a123cb7_7fc3_4180_ae0c_6c4d3c626198

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/abi.h>

namespace ProjectExplorer {
class ToolChain;
}

namespace hammer{ namespace QtCreator{

class HammerTarget;
class HammerBuildConfigurationFactory;

class HammerBuildConfiguration : public ProjectExplorer::BuildConfiguration
{
      Q_OBJECT
      friend class HammerBuildConfigurationFactory;
   
   public:
      HammerBuildConfiguration(HammerTarget *parent);
      ~HammerBuildConfiguration();

      HammerTarget *hammerTarget() const;

      virtual QString buildDirectory() const;

      ProjectExplorer::IOutputParser *createOutputParser() const;

      BuildType buildType() const;

   protected:
      HammerBuildConfiguration(HammerTarget *parent, HammerBuildConfiguration *source);
};

class HammerBuildConfigurationFactory : public ProjectExplorer::IBuildConfigurationFactory
{
      Q_OBJECT

   public:
      HammerBuildConfigurationFactory(QObject *parent = 0);
      ~HammerBuildConfigurationFactory();

      QStringList availableCreationIds(ProjectExplorer::Target *parent) const;
      QString displayNameForId(const QString &id) const;

      bool canCreate(ProjectExplorer::Target *parent, const QString &id) const;
      HammerBuildConfiguration *create(ProjectExplorer::Target *parent, const QString &id);
      bool canClone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source) const;
      HammerBuildConfiguration *clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source);
      bool canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const;
      HammerBuildConfiguration *restore(ProjectExplorer::Target *parent, const QVariantMap &map);
};

}} 

#endif //h_7a123cb7_7fc3_4180_ae0c_6c4d3c626198
