#if !defined(h_59138cbc_7f28_4b6b_877c_f25a35612a44)
#define h_59138cbc_7f28_4b6b_877c_f25a35612a44

#include <coreplugin/editormanager/ieditorfactory.h>
#include "hammerprojectmanager.h"
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE

namespace TextEditor {
   class TextEditorActionHandler;
}

namespace hammer{ namespace QtCreator{

class HammerEditorFactory : public Core::IEditorFactory
{
    Q_OBJECT

public:
    HammerEditorFactory(ProjectManager *parent, TextEditor::TextEditorActionHandler *handler);
    ~HammerEditorFactory();

    virtual QStringList mimeTypes() const;
    virtual Core::Id id() const;
    virtual QString displayName() const;
    Core::IFile *open(const QString &fileName);
    Core::IEditor *createEditor(QWidget *parent);

private:
    const QStringList m_mimeTypes;
    ProjectManager *m_manager;
    TextEditor::TextEditorActionHandler *m_actionHandler;
};

}}
#endif //h_59138cbc_7f28_4b6b_877c_f25a35612a44
