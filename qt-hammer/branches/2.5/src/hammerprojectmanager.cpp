#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>
#include <projectexplorer/session.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/idocument.h>
#include <coreplugin/icore.h>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <hammer/core/toolset_manager.h>
#include <hammer/core/toolsets/gcc_toolset.h>
#include <hammer/core/toolsets/msvc_toolset.h>
#include <hammer/core/toolsets/qt_toolset.h>
#include <hammer/core/types.h>
#include <hammer/core/generator_registry.h>
#include <hammer/core/copy_generator.h>
#include <hammer/core/obj_generator.h>
#include <hammer/core/testing_generators.h>
#include <hammer/core/header_lib_generator.h>
#include <hammer/core/c_scanner.h>
#include <hammer/core/scaner_manager.h>
#include <hammer/core/feature_set.h>
#include <hammer/core/main_target.h>
#include <hammer/core/target_type.h>
#include <hammer/core/fs_helpers.h>

#include <QFileInfo>
#include <QAction>

#include "hammerprojectmanager.h"
#include "hammerproject.h"
#include "hammerprojectnode.h"
#include "hammerprojectconstants.h"
#include "visibleprojectsdialog.h"

using namespace std;

namespace hammer{namespace QtCreator{

static void use_toolset_rule(project*, engine& e, pstring& toolset_name, pstring& toolset_version, pstring* toolset_home_)
{
   location_t toolset_home;
   if (toolset_home_ != NULL)
      toolset_home = toolset_home_->to_string();

   e.toolset_manager().init_toolset(e, toolset_name.to_string(), toolset_version.to_string(), toolset_home_ == NULL ? NULL : &toolset_home);
}

ProjectManager::ProjectManager()
   : show_visible_dialog_on_open_(true)
{
#if defined(_WIN32)
   m_engine.load_hammer_script("D:\\bin\\hammer\\scripts\\startup.ham");
#else
   m_engine.load_hammer_script("/usr/lib/hammer/scripts/startup.ham");
#endif
   types::register_standart_types(m_engine.get_type_registry(), m_engine.feature_registry());
   m_engine.generators().insert(std::auto_ptr<generator>(new copy_generator(m_engine)));
   m_engine.generators().insert(std::auto_ptr<generator>(new obj_generator(m_engine)));
   add_testing_generators(m_engine, m_engine.generators());
   add_header_lib_generator(m_engine, m_engine.generators());

   boost::shared_ptr<scanner> c_scaner(new hammer::c_scanner);
   m_engine.scanner_manager().register_scanner(m_engine.get_type_registry().get(types::CPP), c_scaner);
   m_engine.scanner_manager().register_scanner(m_engine.get_type_registry().get(types::C), c_scaner);
   m_engine.scanner_manager().register_scanner(m_engine.get_type_registry().get(types::RC), c_scaner);

   m_engine.toolset_manager().add_toolset(auto_ptr<toolset>(new msvc_toolset));
   m_engine.toolset_manager().add_toolset(auto_ptr<toolset>(new gcc_toolset));
   m_engine.toolset_manager().add_toolset(auto_ptr<toolset>(new qt_toolset));

   m_engine.call_resolver().insert("use-toolset", boost::function<void (project*, pstring&, pstring&, pstring*)>(boost::bind(use_toolset_rule, _1, boost::ref(m_engine), _2, _3, _4)));
   m_engine.toolset_manager().autoconfigure(m_engine);

   ProjectExplorer::ProjectExplorerPlugin* projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();

   connect(projectExplorer->session(),
           SIGNAL(aboutToLoadSession(QString)),
           SLOT(on_aboutToLoadSession(QString)));

   connect(projectExplorer->session(),
           SIGNAL(sessionLoaded(QString)),
           SLOT(on_sessionLoaded(QString)));

}

QString ProjectManager::mimeType() const
{
   return QLatin1String(HAMMERMIMETYPE);
}

void gatherAllMainTargets(boost::unordered_set<const main_target*>& targets,
                          const main_target& targetToInspect)
{
   if (targets.find(&targetToInspect) != targets.end())
      return;

   targets.insert(&targetToInspect);

   BOOST_FOREACH(const basic_target* bt, targetToInspect.sources())
         gatherAllMainTargets(targets, *bt->get_main_target());

   BOOST_FOREACH(const basic_target* bt, targetToInspect.dependencies())
         gatherAllMainTargets(targets, *bt->get_main_target());
}

ProjectExplorer::Project *ProjectManager::openProject(const QString &fileName, QString *errorString)
{
   if (!QFileInfo(fileName).isFile())
      return NULL;

   ProjectExplorer::ProjectExplorerPlugin *projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();
   foreach (ProjectExplorer::Project *pi, projectExplorer->session()->projects()) {
      if (fileName == pi->document()->fileName()) {
         Core::MessageManager *messageManager = Core::ICore::instance()->messageManager();
         messageManager->printToOutputPanePopup(tr("Failed opening project '%1': Project already open")
                                                .arg(QDir::toNativeSeparators(fileName)));
         return NULL;
      }
   }

   hammer::project* opened_project = NULL;
   // load hammer project
   try {
      opened_project = &get_engine().load_project(resolve_symlinks(location_t(fileName.toStdString())).branch_path());
   } catch(const std::exception& e) {
      Core::MessageManager *messageManager = Core::ICore::instance()->messageManager();
      messageManager->printToOutputPanePopup(tr("Failed opening project '%1': %2")
                                             .arg(QDir::toNativeSeparators(fileName)).arg(e.what()));

      return NULL;
   }

   // find out which target to build
   const basic_meta_target* target = NULL;
   for(hammer::project::targets_t::const_iterator i = opened_project->targets().begin(), last = opened_project->targets().end(); i != last; ++i) {
      if (!i->second->is_explicit()) {
         if (target != NULL) {
            Core::MessageManager *messageManager = Core::ICore::instance()->messageManager();
            messageManager->printToOutputPanePopup(tr("Failed opening project '%1': Project contains more than one implicit target")
                                                   .arg(QDir::toNativeSeparators(fileName)));

            return NULL;
         } else
            target = i->second;
      }
   }

   // create environment for instantiation
   feature_set* build_request = m_engine.feature_registry().make_set();
   feature_set* usage_requirements = m_engine.feature_registry().make_set();

#if defined(_WIN32)
   if (build_request->find("toolset") == build_request->end())
      build_request->join("toolset", "msvc");
#else
   if (build_request->find("toolset") == build_request->end())
      build_request->join("toolset", "gcc");
#endif

   if (build_request->find("variant") == build_request->end())
      build_request->join("variant", "debug");

   if (build_request->find("host-os") == build_request->end())
      build_request->join("host-os", m_engine.feature_registry().get_def("host-os").get_default().c_str());

   // instantiate selected target
   vector<basic_target*> instantiated_targets;
   try {
      target->instantiate(NULL, *build_request, &instantiated_targets, usage_requirements);
   } catch(const std::exception& e) {
      Core::MessageManager *messageManager = Core::ICore::instance()->messageManager();
      messageManager->printToOutputPanePopup(tr("Failed opening project '%1': %2")
                                             .arg(QDir::toNativeSeparators(fileName)).arg(e.what()));

      return NULL;
   }

   if (instantiated_targets.size() != 1) {
      Core::MessageManager *messageManager = Core::ICore::instance()->messageManager();
      messageManager->printToOutputPanePopup(tr("Failed opening project '%1': Target instantiation produce more than one result")
                                             .arg(QDir::toNativeSeparators(fileName)));

      return NULL;
   }

   const main_target* topMainTarget = dynamic_cast<main_target*>(instantiated_targets[0]);

   if (show_visible_dialog_on_open_) {
      visible_targets_t new_targets(gather_all_targets(*topMainTarget));
      visible_targets_t fresh_targets;
      std::set_difference(new_targets.begin(), new_targets.end(),
                          visible_targets_.begin(), visible_targets_.end(),
                          std::inserter(fresh_targets, fresh_targets.end()));

      VisibleProjectsDialog dlg(fresh_targets);
      if (dlg.exec() == QDialog::Accepted)
         visible_targets_.insert(dlg.targets_.begin(), dlg.targets_.end());
   }

   HammerProject* mainProject = new HammerProject(this, topMainTarget, /*main_project=*/true);
   return mainProject;
}

void ProjectManager::registerProject(HammerProject* project)
{
   m_projects.append(project);
}

void ProjectManager::unregisterProject(HammerProject* project)
{
   m_projects.removeAll(project);
}

ProjectExplorer::ProjectNode*
ProjectManager::add_dep(const main_target& mt, const HammerProject& owner)
{
   visible_targets_t::const_iterator vi = visible_targets_.find(&mt);
   if (vi != visible_targets_.end() && !vi->second)
      return NULL;

   stored_visible_targets_t::const_iterator svi = stored_visible_targets_.find(mt.location().string());
   if (svi != stored_visible_targets_.end())
      visible_targets_.insert(make_pair(&mt, svi->second));

   if (svi != stored_visible_targets_.end() && !svi->second)
      return NULL;


   deps_t::const_iterator i = deps_.find(&mt);
   if (i != deps_.end()) {
      HammerDepLinkProjectNode* result = new HammerDepLinkProjectNode(*i->second);
      return result;
   }

   HammerDepProjectNode* result = new HammerDepProjectNode(mt, owner);
   deps_.insert(make_pair(&mt, result));

   return result;
}

visible_targets_t
ProjectManager::gather_all_targets(const main_target& mt)
{
   boost::unordered_set<const ::hammer::main_target*> targets;
   gatherAllMainTargets(targets, mt);

   visible_targets_t visible_targets;
   std::for_each(targets.cbegin(), targets.cend(), [&](const main_target* mt){
      if (!mt->type().equal_or_derived_from(types::PREBUILT_SHARED_LIB) &&
          !mt->type().equal_or_derived_from(types::PREBUILT_STATIC_LIB) &&
          !mt->type().equal_or_derived_from(types::SEARCHED_LIB) &&
          !mt->type().equal_or_derived_from(types::HEADER_LIB) &&
          !mt->type().equal_or_derived_from(types::PCH) &&
          !mt->type().equal_or_derived_from(types::OBJ) &&
          !mt->type().equal_or_derived_from(qt_uic_main))
      {
         visible_targets.insert(make_pair(mt, true));
      }
   });

   return visible_targets;
}

visible_targets_t
ProjectManager::gather_all_targets(ProjectExplorer::Project* project)
{
   return gather_all_targets(static_cast<HammerProject*>(project)->get_main_target());
}

void ProjectManager::set_visible_targets(const visible_targets_t& v)
{
   visible_targets_ = v;
   deps_.clear();
   for(HammerProject* p : m_projects)
      static_cast<HammerProjectNode*>(p->rootProjectNode())->wipe_nodes();

   for(HammerProject* p : m_projects) {
      static_cast<HammerProjectNode*>(p->rootProjectNode())->refresh();
      p->refresh();
   }
}

void ProjectManager::set_stored_visible_targets(const stored_visible_targets_t& v)
{
   stored_visible_targets_ = v;
}

void ProjectManager::on_aboutToLoadSession(QString sessionName)
{
   show_visible_dialog_on_open_ = false;
}

void ProjectManager::on_sessionLoaded(QString sessionName)
{
   show_visible_dialog_on_open_ = true;
}

}}