#if !defined(h_2d3dca1c_c0f1_4607_b14f_ba5806df07cc)
#define h_2d3dca1c_c0f1_4607_b14f_ba5806df07cc

#include <projectexplorer/abstractprocessstep.h>

class QListWidgetItem;

namespace hammer{ namespace QtCreator{

class HammerBuildConfiguration;
class HammerMakeStepConfigWidget;
class HammerMakeStepFactory;

class HammerMakeStep : public ProjectExplorer::AbstractProcessStep
{
      Q_OBJECT
      friend class HammerMakeStepConfigWidget; // TODO remove again?
      friend class HammerMakeStepFactory;

   public:
      HammerMakeStep(ProjectExplorer::BuildStepList *parent);
      virtual ~HammerMakeStep();

      HammerBuildConfiguration *hammerBuildConfiguration() const;

      virtual bool init();

      virtual void run(QFutureInterface<bool> &fi);

      virtual ProjectExplorer::BuildStepConfigWidget *createConfigWidget();
      virtual bool immutable() const;
      bool buildsTarget(const QString &target) const;
      void setBuildTarget(const QString &target, bool on);
      QString allArguments() const;
      QString makeCommand() const;

      QVariantMap toMap() const;

   protected:
      HammerMakeStep(ProjectExplorer::BuildStepList *parent, HammerMakeStep *bs);
      HammerMakeStep(ProjectExplorer::BuildStepList *parent, const QString &id);
      virtual bool fromMap(const QVariantMap &map);

   private:
      void ctor();

      QStringList m_buildTargets;
      QString m_makeArguments;
      QString m_makeCommand;
};

class HammerMakeCurrentStep : public ProjectExplorer::AbstractProcessStep
{
      Q_OBJECT
      friend class HammerMakeStepConfigWidget; // TODO remove again?
      friend class HammerMakeStepFactory;

   public:
      HammerMakeCurrentStep(ProjectExplorer::BuildStepList *parent);

      HammerBuildConfiguration *hammerBuildConfiguration() const;
      virtual bool init();
      virtual void run(QFutureInterface<bool> &fi);
      virtual ProjectExplorer::BuildStepConfigWidget *createConfigWidget();
      virtual bool immutable() const { return false; }
      void setTargetToBuid(const QString& target, const QString& projectPath);

   protected:
      HammerMakeCurrentStep(ProjectExplorer::BuildStepList *parent, HammerMakeCurrentStep *bs);
   
   private:
      QString m_targetToBuild;
};

class HammerMakeStepConfigWidget :public ProjectExplorer::BuildStepConfigWidget
{
      Q_OBJECT
   public:
      HammerMakeStepConfigWidget(HammerMakeStep *makeStep);
      virtual QString displayName() const;
      virtual QString summaryText() const;
   
   private slots:
      void itemChanged(QListWidgetItem*);
      void makeLineEditTextEdited();
      void makeArgumentsLineEditTextEdited();
      void updateMakeOverrrideLabel();
      void updateDetails();

   private:
//      Ui::GenericMakeStep *m_ui;
      HammerMakeStep *m_makeStep;
      QString m_summaryText;
};

class HammerMakeStepFactory : public ProjectExplorer::IBuildStepFactory
{
      Q_OBJECT
   public:
      explicit HammerMakeStepFactory(QObject *parent = 0);
      virtual ~HammerMakeStepFactory();

      virtual bool canCreate(ProjectExplorer::BuildStepList *parent,
                           const QString &id) const;
      virtual ProjectExplorer::BuildStep *create(ProjectExplorer::BuildStepList *parent,
                                               const QString &id);
      virtual bool canClone(ProjectExplorer::BuildStepList *parent,
                          ProjectExplorer::BuildStep *source) const;
      virtual ProjectExplorer::BuildStep *clone(ProjectExplorer::BuildStepList *parent,
                                              ProjectExplorer::BuildStep *source);
      virtual bool canRestore(ProjectExplorer::BuildStepList *parent,
                            const QVariantMap &map) const;
      virtual ProjectExplorer::BuildStep *restore(ProjectExplorer::BuildStepList *parent,
                                                const QVariantMap &map);

      virtual QStringList availableCreationIds(ProjectExplorer::BuildStepList *bc) const;
      virtual QString displayNameForId(const QString &id) const;
};

}}

#endif //h_2d3dca1c_c0f1_4607_b14f_ba5806df07cc
