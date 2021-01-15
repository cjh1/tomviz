/* This source file is part of the Tomviz project, https://tomviz.org/.
   It is released under the 3-Clause BSD License, see "LICENSE". */

#ifndef tomvizDataBrokerLoadDialog_h
#define tomvizDataBrokerLoadDialog_h

#include <QDialog>

#include <QScopedPointer>
#include <QList>
#include <QVariantMap>

namespace Ui {
class DataBrokerLoadDialog;
}

class QTreeWidgetItem;

namespace tomviz {

class DataBroker;
class ListResourceCall;

class DataBrokerLoadDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DataBrokerLoadDialog(DataBroker* dataBroker, QWidget* parent = nullptr);
  ~DataBrokerLoadDialog() override;

  QString selectedCatalog();
  QString selectedRunUid();
  QString selectedTable();
  QString selectedVariable();

private:
  Q_DISABLE_COPY(DataBrokerLoadDialog)
  QScopedPointer<Ui::DataBrokerLoadDialog> m_ui;
  DataBroker* m_dataBroker;
  QList<QVariantMap> m_catalogs;
  QList<QVariantMap> m_runs;
  QList<QVariantMap> m_tables;
  QList<QVariantMap> m_variables;

  QString m_selectedCatalog;
  QString m_selectedRunUid;
  QString m_selectedTable;
  QString m_selectedVariable;

  void loadCatalogs();
  void loadRuns(const QString& catalog);
  void loadTables(const QString& catalog, const QString& runUid);
  void loadVariables(const QString& catalog, const QString& runUid, const QString& table);

  void showCatalogs();
  void showRuns();
  void showTables();
  void showVariables();
  void setLabel(const QString& label);
  void setEnabledResetButton(bool enable);
  void setEnabledOkButton(bool enable);
  void beginCall();
  void endCall();
  void setErrorMessage(const QString& errorMessage);
  void clearErrorMessage();
  void connectErrorSignal(ListResourceCall *call);
};
} // namespace tomviz

#endif
