/******************************************************************************

  This source file is part of the tomviz project.

  Copyright Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "PassiveAcquisitionWidget.h"

#include "ui_PassiveAcquisitionWidget.h"

#include "AcquisitionClient.h"
#include "ActiveObjects.h"
#include "ConnectionDialog.h"
#include "InterfaceBuilder.h"

#include "DataSource.h"
#include "ModuleManager.h"
#include "Pipeline.h"
#include "PipelineManager.h"

#include <pqApplicationCore.h>
#include <pqSettings.h>
#include <vtkSMProxy.h>

#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkImageProperty.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkInteractorStyleRubberBand2D.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkScalarsToColors.h>
#include <vtkTIFFReader.h>

#include <QBuffer>
#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QNetworkReply>
#include <QProcess>

namespace tomviz {

const char* PASSIVE_ADAPTER = "tomviz.acquisition.vendors.passive.PassiveWatchSource";

PassiveAcquisitionWidget::PassiveAcquisitionWidget(QWidget* parent)
  : QWidget(parent), m_ui(new Ui::PassiveAcquisitionWidget),
    m_client(new AcquisitionClient("http://localhost:8080/acquisition", this)),
    m_connectParamsWidget(new QWidget), m_watchTimer(new QTimer)
{
  m_ui->setupUi(this);
  this->setWindowFlags(Qt::Dialog);

  connect(m_ui->watchPathLineEdit, &QLineEdit::textChanged, [this](const QString text) {
    this->m_ui->watchButton->setEnabled(!text.isEmpty());
  });

  connect(m_ui->watchButton, &QPushButton::clicked, [this]() {
    this->m_retryCount = 5;
    this->connectToServer();
  });

  connect(m_ui->stopWatchingButton, &QPushButton::clicked, [this]() {
    this->m_watchTimer->stop();
    this->m_ui->stopWatchingButton->setEnabled(false);
    this->m_ui->watchButton->setEnabled(true);
  });

  //connect(m_ui->connectButton, SIGNAL(clicked(bool)),
  //  SLOT(connectToServer()));
  //  connect(m_ui->disconnectButton, SIGNAL(clicked(bool)),
  //          SLOT(disconnectFromServer()));
  //  connect(m_ui->previewButton, SIGNAL(clicked(bool)), SLOT(setTiltAngle()));
  //  connect(m_ui->introspectButton, &QPushButton::clicked, [this]() {
  //    this->introspectSource();
  //    m_ui->introspectButton->setEnabled(false);
  //    m_ui->connectButton->setEnabled(true);
  //  });
  //  connect(this, &AcquisitionWidget::connectParameterDescription, this,
  //          &AcquisitionWidget::generateConnectUI);
  //
  //  connect(m_ui->watchButton, &QPushButton::clicked, [this]() {
  //    if (!this->m_watchTimer->isActive()) {
  //      this->watchSource();
  //      m_ui->watchButton->setText("Stop watching");
  //    } else {
  //      this->m_watchTimer->stop();
  //      m_ui->watchButton->setText("Watch");
  //    }
  //
  //  });
  //
  //  m_ui->imageWidget->GetRenderWindow()->AddRenderer(m_renderer.Get());
  //  m_ui->imageWidget->GetInteractor()->SetInteractorStyle(
  //    m_defaultInteractorStyle.Get());
  //  m_defaultInteractorStyle->SetRenderOnMouseMove(true);
  //
  //  m_renderer->SetBackground(1.0, 1.0, 1.0);
  //  m_renderer->SetViewport(0.0, 0.0, 1.0, 1.0);

  readSettings();
}

PassiveAcquisitionWidget::~PassiveAcquisitionWidget() = default;

void PassiveAcquisitionWidget::closeEvent(QCloseEvent* event)
{
  writeSettings();
  event->accept();
}

void PassiveAcquisitionWidget::readSettings()
{
  auto settings = pqApplicationCore::instance()->settings();
  if (!settings->contains("acquisition/geometry")) {
    return;
  }
  //  settings->beginGroup("acquisition");
  //  setGeometry(settings->value("geometry").toRect());
  //  m_ui->splitter->restoreState(settings->value("splitterSizes").toByteArray());
  //  m_ui->hostnameEdit->setText(
  //    settings->value("hostname", "localhost").toString());
  //  m_ui->portEdit->setText(settings->value("port", "8080").toString());
  //  settings->endGroup();
}

QVariantMap PassiveAcquisitionWidget::settings()
{
  QVariantMap settingsMap;
  auto settings = pqApplicationCore::instance()->settings();
  settings->beginGroup("acquisition");
  foreach (QString key, settings->childKeys()) {
    settingsMap[key] = settings->value(key);
  }
  settings->endGroup();

  return settingsMap;
}

void PassiveAcquisitionWidget::writeSettings()
{
  //  auto settings = pqApplicationCore::instance()->settings();
  //  settings->beginGroup("acquisition");
  //  settings->setValue("geometry", geometry());
  //  settings->setValue("splitterSizes", m_ui->splitter->saveState());
  //  settings->setValue("hostname", m_ui->hostnameEdit->text());
  //  settings->setValue("port", m_ui->portEdit->text());
  //  auto connectValues =
  //    InterfaceBuilder::parameterValues(m_connectParamsWidget.data());
  //  for (QVariantMap::const_iterator iter = connectValues.begin();
  //       iter != connectValues.end(); ++iter) {
  //    settings->setValue(iter.key(), iter.value());
  //  }
  //  settings->endGroup();
}

void PassiveAcquisitionWidget::connectToServer(bool startServer)
{
  if (this->m_retryCount == 0) {
    this->displayError("Retry count excceed trying to connect to server.");
    return;
  }

  m_client->setUrl(this->url());
  auto request = m_client->connect(this->connectParams());
  connect(request, &AcquisitionClientRequest::finished, [this]() {
    // Now check that we are connected to server that has the right adapter
    // loaded.
    auto describeRequest = this->m_client->describe();
    connect(describeRequest, &AcquisitionClientRequest::error, this,
        &PassiveAcquisitionWidget::onError);
    connect(describeRequest, &AcquisitionClientRequest::finished, [this](const QJsonValue& result) {
      if (!result.isObject()) {
        this->onError("Invalid response to describe request:", result);
        return;
      }

      if (result.toObject()["name"] != PASSIVE_ADAPTER) {
        this->onError("The server is not running the passive acquisition " \
            "adapter, please restart the server with the correct adapter.",
            QJsonValue());
        return;
      }

      // Now we can start watching.
      this->watchSource();

    });
  });

  connect(request, &AcquisitionClientRequest::error, [startServer, this](const QString& errorMessage,
      const QJsonValue& errorData) {

      auto connection = this->m_ui->connectionsWidget->selectedConnection();
      // If we are getting a connection refused error and we are trying to connect
      // to localhost, try to start the server.
      if (startServer && errorData.toInt() == QNetworkReply::ConnectionRefusedError &&
          connection.hostName() == "localhost") {
        this->startLocalServer();
      }
      else {
        this->onError(errorMessage, errorData);
      }
  });
}

void PassiveAcquisitionWidget::disconnectFromServer()
{
  //  m_ui->statusEdit->setText("Disconnecting");
  //  auto request = m_client->disconnect(QJsonObject());
  //  connect(request, SIGNAL(finished(QJsonValue)), SLOT(onDisconnect()));
  //  connect(request, &AcquisitionClientRequest::error, this,
  //          &PassiveAcquisitionWidget::onError);
}


void PassiveAcquisitionWidget::acquirePreview(const QJsonValue& result)
{
  //  // This should be the actual angle the stage is at.
  //  if (result.isDouble()) {
  //    m_tiltAngle = result.toDouble(-69.99);
  //    m_ui->tiltAngle->setText(QString::number(result.toDouble(-69.99), 'g',
  //    2));
  //  }
  //
  //  auto request = m_client->preview_scan();
  //  connect(request, SIGNAL(finished(QString, QByteArray)),
  //          SLOT(previewReady(QString, QByteArray)));
  //  connect(request, &AcquisitionClientRequest::error, this,
  //          &AcquisitionWidget::onError);
}

void PassiveAcquisitionWidget::previewReady(QString mimeType, QByteArray result)
{
  if (mimeType != "image/tiff") {
    qDebug() << "image/tiff is the only supported mime type right now.\n"
             << mimeType << "\n";
    return;
  }

  QDir dir(QDir::homePath() + "/tomviz-data");
  if (!dir.exists()) {
    dir.mkpath(dir.path());
  }

  QString path = "/tomviz_";

  if (m_tiltAngle > 0.0) {
    path.append('+');
  }
  path.append(QString::number(m_tiltAngle, 'g', 2));
  path.append(".tiff");

  QFile file(dir.path() + path);
  file.open(QIODevice::WriteOnly);
  file.write(result);
  qDebug() << "Data file:" << file.fileName();
  file.close();

  vtkNew<vtkTIFFReader> reader;
  reader->SetFileName(file.fileName().toLatin1());
  reader->Update();
  m_imageData = reader->GetOutput();
  m_imageSlice->GetProperty()->SetInterpolationTypeToNearest();
  m_imageSliceMapper->SetInputData(m_imageData.Get());
  m_imageSliceMapper->Update();
  m_imageSlice->SetMapper(m_imageSliceMapper.Get());
  m_renderer->AddViewProp(m_imageSlice.Get());

  // If we haven't added it, add our live data source to the pipeline.
  if (!m_dataSource) {
    m_dataSource = new DataSource(m_imageData);
    m_dataSource->setLabel("Live!");
    auto pipeline = new Pipeline(m_dataSource);
    PipelineManager::instance().addPipeline(pipeline);
    ModuleManager::instance().addDataSource(m_dataSource);
    pipeline->addDefaultModules(m_dataSource);
  } else {
    m_dataSource->appendSlice(m_imageData);
  }
}

void PassiveAcquisitionWidget::onError(const QString& errorMessage,
                                       const QJsonValue& errorData)
{
  auto message = errorMessage;
  if (!errorData.toString().isEmpty()) {
    message = QString("%1\n%2").arg(message).arg(errorData.toString());
  }

  this->displayError(message);
}

void PassiveAcquisitionWidget::displayError(const QString& errorMessage)
{
  QMessageBox::warning(this, "Acquisition Error",
      errorMessage,
      QMessageBox::Ok);
}

QString PassiveAcquisitionWidget::url() const
{
  auto connection = m_ui->connectionsWidget->selectedConnection();

  return QString("http://%1:%2/acquisition").arg(connection.hostName()).arg(connection.port());
}


void PassiveAcquisitionWidget::watchSource()
{
  this->m_ui->watchButton->setEnabled(false);
  this->m_ui->stopWatchingButton->setEnabled(true);
  connect(this->m_watchTimer, &QTimer::timeout, this,
          [this]() {
            auto request = m_client->stem_acquire();
            connect(request, &AcquisitionClientImageRequest::finished,
                    [this](const QString mimeType, const QByteArray& result,
                           const QJsonObject& meta) {
                      if (!result.isNull()) {
                        qDebug() << "New image received!";
                        this->previewReady(mimeType, result);
                      }
                    });
            connect(request, &AcquisitionClientRequest::error, this,
                    &PassiveAcquisitionWidget::onError);

          },
          Qt::UniqueConnection);
  this->m_watchTimer->start(1000);
}

QJsonObject PassiveAcquisitionWidget::connectParams() {
  QJsonObject connectParams{
    {  "path", m_ui->watchPathLineEdit->text() },
    {  "fileNameRegex", m_ui->fileNameRegexLineEdit->text() },
  };

  auto fileNameRegexGroups = m_ui->regexGroupsWidget->regexGroups();
  if (!fileNameRegexGroups.isEmpty()) {
    auto groups = QJsonArray::fromStringList(fileNameRegexGroups);
    connectParams["fileNameRegexGroups"] = groups;
  }

  return connectParams;
}

void PassiveAcquisitionWidget::startLocalServer() {
  QStringList arguments;
  arguments << "-m" << "tomviz" << "-a" << PASSIVE_ADAPTER << "-r";

  auto serverProcess = new QProcess();
  serverProcess->setProgram("/home/cjh/virtualenv/tomviz_acq/bin/python");
  serverProcess->setArguments(arguments);
  connect(serverProcess, &QProcess::errorOccurred, [this, serverProcess](QProcess::ProcessError error) {
    auto message = QString("Error starting local acquisition: '%1'").arg(serverProcess->errorString());

    QMessageBox::warning(this, "Acquisition Error",
        message,
        QMessageBox::Ok);

  });

  connect(serverProcess, &QProcess::started, [this]() {
    // Now try to connect and watch. Note we are not asking for server to be
    // started if the connection fails, this is to prevent us getting into a
    // connect loop.
    QTimer::singleShot(200, [this](){
      this->m_retryCount--;
      this->connectToServer(false);
    });

  });

  serverProcess->start();
}

}
