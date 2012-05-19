#if !defined(h_5f5d6e1c_be16_4925_829f_765fb4b1374b)
#define h_5f5d6e1c_be16_4925_829f_765fb4b1374b

#include <projectexplorer/customexecutablerunconfiguration.h>
#include <boost/optional.hpp>

namespace hammer{ namespace QtCreator{

class HammerTarget;

class HammerRunConfiguration : public ProjectExplorer::CustomExecutableRunConfiguration
{
      Q_OBJECT
      friend class HammerRunConfigurationFactory;
   public:
      explicit HammerRunConfiguration(HammerTarget *parent);
      virtual QString executable() const;
      virtual Utils::Environment environment() const;
      QVariantMap toMap() const;

   protected:
      HammerRunConfiguration(HammerTarget *parent, HammerRunConfiguration *source);

   private:
      HammerTarget* m_target;
      mutable boost::optional<QString> m_executable;
      mutable boost::optional<QStringList> m_additionalPaths;
};

class HammerRunConfigurationFactory : public ProjectExplorer::IRunConfigurationFactory
{
      Q_OBJECT
   public:
      explicit HammerRunConfigurationFactory(QObject *parent = NULL);
      QStringList availableCreationIds(ProjectExplorer::Target *parent) const;
      QString displayNameForId(const QString &id) const;

      bool canCreate(ProjectExplorer::Target *parent, const QString &id) const;
      ProjectExplorer::RunConfiguration *create(ProjectExplorer::Target *parent, const QString &id);
      bool canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const;
      ProjectExplorer::RunConfiguration *restore(ProjectExplorer::Target *parent, const QVariantMap &map);
      bool canClone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *source) const;
      ProjectExplorer::RunConfiguration *clone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *source);
};

}}

#endif //h_5f5d6e1c_be16_4925_829f_765fb4b1374b
