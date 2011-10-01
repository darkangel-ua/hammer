#include <coreplugin/ifile.h>
#include <QFileInfo>

#include <boost/foreach.hpp>
#include <hammer/core/main_target.h>
#include <hammer/core/types.h>
#include <hammer/core/target_type.h>

#include "hammerprojectnode.h"
#include "hammerproject.h"

namespace hammer{namespace QtCreator{

HammerProjectNode::HammerProjectNode(HammerProject* project, 
                                     Core::IFile* projectFile)
   : ProjectExplorer::ProjectNode(projectFile->fileName()),
     m_project(project),
     m_projectFile(projectFile)
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

void HammerProjectNode::refresh()
{
   removeFileNodes(fileNodes(), this);
   removeFolderNodes(subFolderNodes(), this);
   
   FolderNode* srcNode = NULL;
   FolderNode* incNode = NULL;

   BOOST_FOREACH(const basic_target* bt, m_project->get_main_target().sources())
   {
      if (bt->type().equal_or_derived_from(types::CPP) || 
          bt->type().equal_or_derived_from(types::C))
      {
         ProjectExplorer::FileNode* f = new ProjectExplorer::FileNode(QString::fromStdString((bt->location() / bt->name().to_string()).native_file_string()), ProjectExplorer::SourceType, false);
         if (!srcNode)
         {
            srcNode = new FolderNode("src");
            addFolderNodes(QList<FolderNode*>() << srcNode, this);
         }

         addFileNodes(QList<ProjectExplorer::FileNode*>() << f, srcNode);
      }
      else
         if (bt->type().equal_or_derived_from(types::H))
         {
            ProjectExplorer::FileNode* f = new ProjectExplorer::FileNode(QString::fromStdString((bt->location() / bt->name().to_string()).native_file_string()), ProjectExplorer::HeaderType, false);
            if (!incNode)
            {
               incNode = new FolderNode("include");
               addFolderNodes(QList<FolderNode*>() << incNode, this);
            }

            addFileNodes(QList<ProjectExplorer::FileNode*>() << f, incNode);
         }
   }
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
