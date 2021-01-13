/* This source file is part of the Tomviz project, https://tomviz.org/.
   It is released under the 3-Clause BSD License, see "LICENSE". */

#ifndef tomvizDataBroker_h
#define tomvizDataBroker_h

#include "PythonUtilities.h"

#include <vtkSmartPointer.h>

#include <QString>
#include <QStringList>
#include <QList>
#include <QVariant>

class vtkImageData;

namespace tomviz {
class DataBroker
{
private:
  Python::Module m_dataBrokerModule;

public:
  DataBroker();
  bool installed();
  QStringList catalogs();
  QList<QVariantMap> runs(const QString &catalog);
  QStringList variables(const QString &catalog, const QString &table, const QString &runUid);
  vtkSmartPointer<vtkImageData> loadVariable(const QString &catalog,  const QString &table, const QString &runUid, const QString &variable);

};

}

#endif
