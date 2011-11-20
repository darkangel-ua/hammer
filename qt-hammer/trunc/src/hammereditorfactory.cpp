#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include "hammereditorfactory.h"
#include "hammerprojectconstants.h"
#include <cassert>

namespace hammer{ namespace QtCreator{

HammerEditorFactory::HammerEditorFactory(ProjectManager *manager, TextEditor::TextEditorActionHandler *handler)
    : m_mimeTypes(QStringList() << QLatin1String(HAMMERMIMETYPE)),
      m_manager(manager),
      m_actionHandler(handler)
{

}

HammerEditorFactory::~HammerEditorFactory()
{
}

Core::Id HammerEditorFactory::id() const
{
    return Core::Id(HAMMER_EDITOR_ID);
}

QString HammerEditorFactory::displayName() const
{
    return tr(HAMMER_EDITOR_DISPLAY_NAME);
}

Core::IFile *HammerEditorFactory::open(const QString &fileName)
{
    Core::IEditor *iface = Core::EditorManager::instance()->openEditor(fileName, id());
    return iface ? iface->file() : 0;
}

Core::IEditor *HammerEditorFactory::createEditor(QWidget *parent)
{
/*
   CMakeEditorWidget *rc = new CMakeEditorWidget(parent, this, m_actionHandler);
   TextEditor::TextEditorSettings::instance()->initializeEditor(rc);
   return rc->editor();
*/
   return NULL;
}

QStringList HammerEditorFactory::mimeTypes() const
{
    return m_mimeTypes;
}

}}
