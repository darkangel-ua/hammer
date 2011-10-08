#include <coreplugin/ifile.h>
#include <QFileInfo>

#include <boost/foreach.hpp>
#include <hammer/core/main_target.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>
#include <hammer/core/toolsets/qt_toolset.h>

#include "hammerprojectnode.h"
#include "hammerproject.h"

namespace hammer{namespace QtCreator{

HammerProjectNode::HammerProjectNode(HammerProject* project, 
                                     Core::IFile* projectFile)
   : ProjectExplorer::ProjectNode(projectFile->fileName()),
     m_project(project),
     m_projectFile(projectFile),
     m_srcNode(NULL),
     m_incNode(NULL),
     m_resNode(NULL),
     m_formNode(NULL)
{
   setDisplayName(QString::fromStdString(project->get_main_target().name().to_string()));
   refresh();
}

Core::IFile *HammerProjectNode::projectFile() const
{
    return m_projectFile;
}

QString HammerProjectNode::projectFilePath() const
{
    return m_projectFile->fileName();
}

void HammerProjectNode::addNodes(const basic_target* bt)
{
   if (bt->type().equal_or_derived_from(types::CPP) ||
       bt->type().equal_or_derived_from(types::C))
   {
      ProjectExplorer::FileNode* f = new ProjectExplorer::FileNode(QString::fromStdString(bt->full_path().string()), ProjectExplorer::SourceType, false);
      if (!m_srcNode)
      {
         m_srcNode = new FolderNode("src");
         addFolderNodes(QList<FolderNode*>() << m_srcNode, this);
      }

      addFileNodes(QList<ProjectExplorer::FileNode*>() << f, m_srcNode);
   }
   else
      if (bt->type().equal_or_derived_from(types::H))
      {
         ProjectExplorer::FileNode* f = new ProjectExplorer::FileNode(QString::fromStdString(bt->full_path().string()), ProjectExplorer::HeaderType, false);
         if (!m_incNode)
         {
            m_incNode = new FolderNode("include");
            addFolderNodes(QList<FolderNode*>() << m_incNode, this);
         }

         addFileNodes(QList<ProjectExplorer::FileNode*>() << f, m_incNode);
      }
      else
         if (bt->type().equal_or_derived_from(hammer::qt_ui))
         {
            ProjectExplorer::FileNode* f = new ProjectExplorer::FileNode(QString::fromStdString(bt->full_path().string()), ProjectExplorer::FormType, false);
            if (!m_formNode)
            {
               m_formNode = new FolderNode("forms");
               addFolderNodes(QList<FolderNode*>() << m_formNode, this);
            }
            addFileNodes(QList<ProjectExplorer::FileNode*>() << f, m_formNode);
         }
         else
            if (bt->type().equal_or_derived_from(hammer::qt_rc))
            {
               ProjectExplorer::FileNode* f = new ProjectExplorer::FileNode(QString::fromStdString(bt->full_path().string()), ProjectExplorer::ResourceType, false);
               if (!m_resNode)
               {
                  m_resNode = new FolderNode("resources");
                  addFolderNodes(QList<FolderNode*>() << m_resNode, this);
               }
               addFileNodes(QList<ProjectExplorer::FileNode*>() << f, m_resNode);
            }
            else
               if (bt->type().equal_or_derived_from(hammer::qt_uic_main) ||
                   bt->type().equal_or_derived_from(hammer::types::PCH) ||
                   bt->type().equal_or_derived_from(hammer::types::OBJ))
                  BOOST_FOREACH(const basic_target* i, static_cast<const main_target*>(bt)->sources())
                        addNodes(i);
}

void HammerProjectNode::refresh()
{
   removeFileNodes(fileNodes(), this);
   removeFolderNodes(subFolderNodes(), this);

   BOOST_FOREACH(const basic_target* bt, m_project->get_main_target().sources())
         addNodes(bt);
}

bool HammerProjectNode::hasBuildTargets() const
{
   return true;
}

QList<ProjectExplorer::ProjectNode::ProjectAction> HammerProjectNode::supportedActions(Node*) const
{
    return QList<ProjectAction>();
}

bool HammerProjectNode::canAddSubProject(const QString&) const
{
    return false;
}

bool HammerProjectNode::addSubProjects(const QStringList&)
{
    return false;
}

bool HammerProjectNode::removeSubProjects(const QStringList&)
{
    return false;
}

bool HammerProjectNode::addFiles(const ProjectExplorer::FileType,
                                 const QStringList&, 
                                 QStringList*)
{
   return false;
}

bool HammerProjectNode::removeFiles(const ProjectExplorer::FileType fileType,
                                    const QStringList &filePaths, QStringList *notRemoved)
{
    return false;
}

bool HammerProjectNode::deleteFiles(const ProjectExplorer::FileType,
                                    const QStringList&)
{
    return false;
}

bool HammerProjectNode::renameFile(const ProjectExplorer::FileType,
                                   const QString&,
                                   const QString&)
{
    return false;
}

QList<ProjectExplorer::RunConfiguration *> HammerProjectNode::runConfigurationsFor(Node*)
{
    return QList<ProjectExplorer::RunConfiguration *>();
}

}}
