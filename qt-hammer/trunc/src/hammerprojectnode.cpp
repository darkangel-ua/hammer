#include <coreplugin/ifile.h>
#include <QFileInfo>

#include <hammer/core/main_target.h>

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
