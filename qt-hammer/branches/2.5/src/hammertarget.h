#if !defined(h_2339c647_1fa9_4edd_ae93_892dd65c7300)
#define h_2339c647_1fa9_4edd_ae93_892dd65c7300

#include <projectexplorer/target.h>

#include <QtCore/QStringList>
#include <QtCore/QVariantMap>
#include <boost/optional.hpp>

namespace ProjectExplorer {
class IBuildConfigurationFactory;
}

namespace hammer{ namespace QtCreator{

const char * const HAMMER_DESKTOP_TARGET_ID("HammerProjectManager.HammerTarget");

class HammerProject;
class HammerBuildConfigurationFactory;

class HammerTargetFactory;

class HammerTarget : public ProjectExplorer::Target
{
    Q_OBJECT
    friend class HammerTargetFactory;

   public:
      explicit HammerTarget(HammerProject *parent);

      ProjectExplorer::BuildConfigWidget *createConfigWidget();

      HammerProject *hammerProject() const;

      ProjectExplorer::IBuildConfigurationFactory *buildConfigurationFactory() const;
      ProjectExplorer::BuildConfiguration* activeBuildConfiguration() const;

   protected:
      bool fromMap(const QVariantMap &map);

   private:
      HammerBuildConfigurationFactory *m_buildConfigurationFactory;

};

class HammerTargetFactory : public ProjectExplorer::ITargetFactory
{
      Q_OBJECT

   public:
      explicit HammerTargetFactory(QObject *parent = 0);

      QStringList supportedTargetIds() const;
      bool supportsTargetId(const QString &id) const;

      QStringList supportedTargetIds(ProjectExplorer::Project *parent) const;
      QString displayNameForId(const QString &id) const;

      bool canCreate(ProjectExplorer::Project *parent, const QString &id) const;
      HammerTarget *create(ProjectExplorer::Project *parent, const QString &id);
      bool canRestore(ProjectExplorer::Project *parent, const QVariantMap &map) const;
      HammerTarget *restore(ProjectExplorer::Project *parent, const QVariantMap &map);
};

}}

#endif //h_2339c647_1fa9_4edd_ae93_892dd65c7300
