/* This source file is part of the Tomviz project, https://tomviz.org/.
   It is released under the 3-Clause BSD License, see "LICENSE". */

#include "DataBrokerLoadReaction.h"
#include "LoadDataReaction.h"

#include "DataSource.h"
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

DataSource* DataBrokerLoadReaction::loadData()
{
  DataBroker dataBroker;
  auto imageData = dataBroker.loadVariable("test", "1b0b4d73-6d87-43ab-8d62-ed035c51b9b4", "primary", "Andor_image");
  auto dataSource = new DataSource(imageData);
  LoadDataReaction::dataSourceAdded(dataSource, true, false);

  return dataSource;
}

}
