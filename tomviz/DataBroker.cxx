/* This source file is part of the Tomviz project, https://tomviz.org/.
   It is released under the 3-Clause BSD License, see "LICENSE". */

#include "DataBroker.h"

#include "DataSource.h"
#include "Utilities.h"

#include <vtkImageData.h>

#include <QDebug>

namespace tomviz {


DataBroker::DataBroker()
{
  Python python;
  m_dataBrokerModule = python.import("tomviz.io._databroker");
  if (!m_dataBrokerModule.isValid()) {
    qCritical() << "Failed to import tomviz.io._databroker module.";
  }
}

bool DataBroker::installed() {
  Python python;

  auto installed = m_dataBrokerModule.findFunction("installed");
  if (!installed.isValid()) {
    qCritical() << "Failed to import tomviz.io._databroker.installed";
    return false;
  }

  auto res = installed.call();

   if (!res.isValid()) {
    qCritical("Error calling installed");
    return false;
  }

  return res.toBool();
}

QStringList DataBroker::catalogs() {
  QStringList catalogs;
  Python python;

  auto catalogsFunc = m_dataBrokerModule.findFunction("catalogs");
  if (!catalogsFunc.isValid()) {
    qCritical() << "Failed to import tomviz.io._databroker.catalogs";
    return catalogs;
  }

  auto res = catalogsFunc.call();

   if (!res.isValid()) {
    qCritical("Error calling catalogs");
    return catalogs;
  }

  auto pyList = res.toList();

  if (!pyList.isValid()) {
    qCritical() << "Error calling toList";
    return catalogs;
  }

  for (auto i = 0; i < pyList.length(); i++) {
    catalogs.append(pyList[i].toString());
  }

  return catalogs;
}

QList<QVariantMap> DataBroker::runs(const QString &catalog) {
  Python python;

  auto runsFunc = m_dataBrokerModule.findFunction("runs");
  if (!runsFunc.isValid()) {
    qCritical() << "Failed to import tomviz.io._databroker.runs";
    return QList<QVariantMap>();
  }

  Python::Tuple args(1);
  args.set(0, catalog.toStdString());

  auto res = runsFunc.call(args);

   if (!res.isValid()) {
    qCritical("Error calling runs");
    return QList<QVariantMap>();
  }

  QList<QVariantMap> runs;

  for(auto v: toQVariant(res.toVariant()).toList()) {
    runs.append(v.toMap());
  }


  return runs;
}

QStringList DataBroker::variables(const QString &catalog, const QString &table, const QString &runUid)
{
  Python python;

  QStringList variables;
  auto variablesFunc = m_dataBrokerModule.findFunction("variables");
  if (!variablesFunc.isValid()) {
    qCritical() << "Failed to import tomviz.io._databroker.variables";
    return variables;
  }

  Python::Tuple args(3);
  args.set(0, catalog.toStdString());
  args.set(1, table.toStdString());
  args.set(2, runUid.toStdString());

  std::cout << "calling\n";
  auto res = variablesFunc.call(args);


  if (!res.isValid()) {
    qCritical("Error calling variables");
    return variables;
  }


  std::cout << "called\n";

  auto pyList = res.toList();


  std::cout << "list\n";
  for (auto i = 0; i < pyList.length(); i++) {
    variables.append(pyList[i].toString());
  }

  return variables;
}

vtkSmartPointer<vtkImageData> DataBroker::loadVariable(const QString &catalog,  const QString &runUid, const QString &table, const QString &variable)
{
  Python python;

  auto loadFunc = m_dataBrokerModule.findFunction("load_variable");
  if (!loadFunc.isValid()) {
    qCritical() << "Failed to import tomviz.io._databroker.load_variable";
    return nullptr;
  }

  Python::Tuple args(4);
  args.set(0, catalog.toStdString());
  args.set(1, runUid.toStdString());
  args.set(2, table.toStdString());
  args.set(3, variable.toStdString());

  auto res = loadFunc.call(args);

  if (!res.isValid()) {
    qCritical("Error calling load_variable");
    return nullptr;
  }

  vtkObjectBase* vtkobject =
    Python::VTK::GetPointerFromObject(res, "vtkImageData");
  if (vtkobject == nullptr) {
    return nullptr;
  }

  vtkSmartPointer<vtkImageData> imageData =
    vtkImageData::SafeDownCast(vtkobject);

  if (imageData->GetNumberOfPoints() <= 1) {
    qCritical() << "The file didn't contain any suitable data";
    return nullptr;
  }

  return imageData;



}

}
