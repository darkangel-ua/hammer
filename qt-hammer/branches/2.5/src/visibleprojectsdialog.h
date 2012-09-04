#ifndef VISIBLEPROJECTSDIALOG_H
#define VISIBLEPROJECTSDIALOG_H

#include <QDialog>
#include <map>
#include <utility>
#include <string>

namespace Ui {
class VisibleProjectsDialog;
}

class QStandardItem;

namespace hammer{ class main_target; }

class VisibleProjectsDialog : public QDialog
{
      Q_OBJECT

   public:
      typedef std::map<const ::hammer::main_target*, bool> visible_targets_t;

      explicit VisibleProjectsDialog(const visible_targets_t& targets, QWidget *parent = 0);
      ~VisibleProjectsDialog();

      visible_targets_t targets_;

   private slots:
      void targets_list_itemChanged(QStandardItem* item);

   private:
      Ui::VisibleProjectsDialog *ui;
};

#endif // VISIBLEPROJECTSDIALOG_H
