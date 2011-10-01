#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <texteditor/texteditoractionhandler.h>

#include <QStringList>
#include <QtPlugin>
#include "hammerplugin.h"
#include "hammerprojectmanager.h"
#include "hammereditorfactory.h"
#include "hammerprojectconstants.h"
#include "hammertarget.h"
#include "hammermakestep.h"

HammerPlugin::HammerPlugin()
{
}

HammerPlugin::~HammerPlugin()
{
}

bool HammerPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)

    Core::ICore *core = Core::ICore::instance();
    Core::MimeDatabase *mimeDB = core->mimeDatabase();

    const QLatin1String mimetypesXml(":hammerproject/HammerProject.mimetypes.xml");

    if (! mimeDB->addMimeTypes(mimetypesXml, errorMessage))
       return false;

    hammer::QtCreator::ProjectManager *manager = new hammer::QtCreator::ProjectManager();
    addAutoReleasedObject(manager);
    addAutoReleasedObject(new hammer::QtCreator::HammerTargetFactory);
    addAutoReleasedObject(new hammer::QtCreator::HammerMakeStepFactory);

//     TextEditor::TextEditorActionHandler *editorHandler
//        = new TextEditor::TextEditorActionHandler(hammer::QtCreator::C_HAMMEREDITOR);
//     addAutoReleasedObject(new hammer::QtCreator::HammerEditorFactory(manager, editorHandler));

    return true;
}

void HammerPlugin::extensionsInitialized()
{
}

Q_EXPORT_PLUGIN(HammerPlugin)
