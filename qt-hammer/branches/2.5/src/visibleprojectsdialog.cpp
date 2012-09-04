#include "visibleprojectsdialog.h"
#include "ui_visibleprojectsdialog.h"
#include <QStandardItemModel>
#include <hammer/core/main_target.h>

Q_DECLARE_METATYPE(VisibleProjectsDialog::visible_targets_t::value_type*);

VisibleProjectsDialog::VisibleProjectsDialog(const visible_targets_t& targets,
                                             QWidget *parent)
   : QDialog(parent),
     targets_(targets),
     ui(new Ui::VisibleProjectsDialog)
{
   ui->setupUi(this);
   QStandardItemModel* m = new QStandardItemModel(this);
   for(visible_targets_t::iterator i = targets_.begin(), last = targets_.end(); i != last; ++i) {
      QString title = QString::fromStdString(i->first->name().to_string());
      QStandardItem* item = new QStandardItem(title);
      item->setCheckable(true);
      if (i->second) item->setCheckState(Qt::Checked);
      item->setData(QVariant::fromValue(&*i));
      m->appendRow(item);
   }
   m->sort(0);

   ui->targets_list_->setModel(m);
   connect(m, SIGNAL(itemChanged(QStandardItem*)), SLOT(targets_list_itemChanged(QStandardItem*)));
}

void VisibleProjectsDialog::targets_list_itemChanged(QStandardItem* item)
{
   visible_targets_t::value_type* i = qvariant_cast<visible_targets_t::value_type*>(item->data());
   i->second = item->checkState() == Qt::Checked;
}

VisibleProjectsDialog::~VisibleProjectsDialog()
{
   delete ui;
}
