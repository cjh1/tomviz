/* This source file is part of the Tomviz project, https://tomviz.org/.
   It is released under the 3-Clause BSD License, see "LICENSE". */

#include "DataBrokerLoadReaction.h"
#include "DataBrokerLoadDialog.h"


#include "DataSource.h"
#include "Utilities.h"

#include <vtkImageData.h>

namespace tomviz {

DataBrokerLoadReaction::DataBrokerLoadReaction(QAction* parentObject)
  : pqReaction(parentObject)
{
}

DataBrokerLoadReaction::~DataBrokerLoadReaction() = default;

void DataBrokerLoadReaction::onTriggered()
{
  loadData();
}

void DataBrokerLoadReaction::loadData()
{
  auto dataBroker = new DataBroker(tomviz::mainWidget());
  auto dialog = new DataBrokerLoadDialog(dataBroker, tomviz::mainWidget());
  dialog->exec();
  /*
  auto dataBroker = new DataBroker();
  auto call = dataBroker->loadVariable("test", "1b0b4d73-6d87-43ab-8d62-ed035c51b9b4", "primary", "Andor_image");
  connect(call, &LoadDataCall::complete, dataBroker, [dataBroker](vtkSmartPointer<vtkImageData> imageData) {
    auto dataSource = new DataSource(imageData);
    dataSource->setLabel("db://test/1b0b4d73/primary/Andor_image");
    LoadDataReaction::dataSourceAdded(dataSource, true, false);
    dataBroker->deleteLater();
  });
  */
}

}
