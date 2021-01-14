/* This source file is part of the Tomviz project, https://tomviz.org/.
   It is released under the 3-Clause BSD License, see "LICENSE". */

#include "DataBrokerLoadDialog.h"
#include "DataBroker.h"

#include "ui_DataBrokerLoadDialog.h"


#include <QTreeWidget>
#include <QDebug>

namespace tomviz {

DataBrokerLoadDialog::DataBrokerLoadDialog(DataBroker* dataBroker, QWidget* parent)
  : QDialog(parent), m_ui(new Ui::DataBrokerLoadDialog), m_dataBroker(dataBroker)
{
  m_ui->setupUi(this);
  m_ui->treeWidget->setExpandsOnDoubleClick(false);

  loadCatalogs();
}



DataBrokerLoadDialog::~DataBrokerLoadDialog() = default;


void DataBrokerLoadDialog::loadCatalogs() {
  setCursor(Qt::WaitCursor);

  auto call = m_dataBroker->catalogs();
  connect(call, &ListResourceCall::complete, call, [this, call](QList<QVariantMap> catalogs) {
    this->m_catalogs = catalogs;
    this->showCatalogs();
    this->unsetCursor();
    call->deleteLater();

  });
}

void DataBrokerLoadDialog::showCatalogs() {
  auto tree = m_ui->treeWidget;
  tree->clear();
  disconnect(tree, &QTreeWidget::itemDoubleClicked, nullptr, nullptr);

  connect(tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item, int column) {
    m_selectedCatalog = item->data(0, Qt::DisplayRole).toString();
    this->loadRuns(m_selectedCatalog);
  });

  tree->setColumnCount(2);

  QStringList headers;
  headers.append("Name");
  headers.append("Description");
  tree->setHeaderLabels(headers);

  QList<QTreeWidgetItem *> items;
  for (auto &cat: m_catalogs) {
      QStringList row;
      row.append(cat["name"].toString());
      row.append(cat["descriptions"].toString());
      items.append(new QTreeWidgetItem(tree, row));
  }
  tree->insertTopLevelItems(0, items);
  std::cout << "done\n";
}

void DataBrokerLoadDialog::loadRuns(const QString& catalog) {
  setCursor(Qt::WaitCursor);

  auto call = m_dataBroker->runs(catalog);
  connect(call, &ListResourceCall::complete, call, [this, call](QList<QVariantMap> runs) {
    this->m_runs = runs;
    this->showRuns();
    this->unsetCursor();
    call->deleteLater();
  });
}

void DataBrokerLoadDialog::showRuns() {
  auto tree = m_ui->treeWidget;
  tree->clear();
  disconnect(tree, &QTreeWidget::itemDoubleClicked, nullptr, nullptr);

  connect(tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item, int column) {
    m_selectedRunUid = item->data(0, Qt::DisplayRole).toString();
    this->loadTables(m_selectedCatalog, m_selectedRunUid);
  });

  tree->setColumnCount(3);

  QStringList headers;
  headers.append("UID");
  headers.append("Name");
  headers.append("Date");
  tree->setHeaderLabels(headers);

  QList<QTreeWidgetItem *> items;
  for (auto run: m_runs) {
      QStringList row;
      row.append(run["uid"].toString());
      row.append(run["name"].toString());
      row.append(run["time"].toString());
      items.append(new QTreeWidgetItem(tree, row));
  }
  tree->insertTopLevelItems(0, items);
  std::cout << "done\n";
}

void DataBrokerLoadDialog::loadTables(const QString& catalog, const QString& runUid) {
  setCursor(Qt::WaitCursor);

  auto call = m_dataBroker->tables(catalog, runUid);
  connect(call, &ListResourceCall::complete, call, [this, call](QList<QVariantMap> tables) {
    this->m_tables = tables;
    this->showTables();
    this->unsetCursor();
    call->deleteLater();
  });
}

void DataBrokerLoadDialog::showTables() {
  auto tree = m_ui->treeWidget;
  tree->clear();
  disconnect(tree, &QTreeWidget::itemDoubleClicked, nullptr, nullptr);

  connect(tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item, int column) {
    m_selectedTable = item->data(0, Qt::DisplayRole).toString();
    this->loadVariables(m_selectedCatalog, m_selectedRunUid, m_selectedTable);
  });

  tree->setColumnCount(1);

  QStringList headers;
  headers.append("Name");
  headers.append("Date");
  tree->setHeaderLabels(headers);

  QList<QTreeWidgetItem *> items;
  for (auto &table: m_tables) {
      QStringList row;
      row.append(table["name"].toString());
      items.append(new QTreeWidgetItem(tree, row));
  }
  tree->insertTopLevelItems(0, items);
}

void DataBrokerLoadDialog::loadVariables(const QString& catalog, const QString& runUid, const QString& table) {
  setCursor(Qt::WaitCursor);

  auto call = m_dataBroker->variables(catalog, runUid, table);
  connect(call, &ListResourceCall::complete, call, [this, call](QList<QVariantMap> variables) {
    this->m_variables = variables;
    this->showVariables();
    this->unsetCursor();
    call->deleteLater();
  });
}

void DataBrokerLoadDialog::showVariables() {
  auto tree = m_ui->treeWidget;
  tree->clear();
  disconnect(tree, &QTreeWidget::itemDoubleClicked, nullptr, nullptr);

  connect(tree, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem *item, int column) {
    m_selectedVariable = item->data(0, Qt::DisplayRole).toString();
  });

  tree->setColumnCount(1);

  QStringList headers;
  headers.append("Name");
  tree->setHeaderLabels(headers);

  QList<QTreeWidgetItem *> items;
  for(auto &v: m_variables) {
      QStringList row;
      row.append(v["name"].toString());
      items.append(new QTreeWidgetItem(tree, row));
  }
  tree->insertTopLevelItems(0, items);
}


} // namespace tomviz
