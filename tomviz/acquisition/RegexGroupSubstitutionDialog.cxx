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

#include "RegexGroupSubstitutionDialog.h"
#include "ui_RegexGroupSubstitutionDialog.h"

namespace tomviz {

RegexGroupSubstitutionDialog::RegexGroupSubstitutionDialog(
  const QString groupName, const QString regex, const QString substitution,
  QWidget* parent)
  : QDialog(parent), m_ui(new Ui::RegexGroupSubstitutionDialog)
{
  m_ui->setupUi(this);

  m_ui->groupNameLineEdit->setText(groupName);
  m_ui->regexLineEdit->setText(regex);
  m_ui->substitutionLineEdit->setText(substitution);
}

RegexGroupSubstitutionDialog::~RegexGroupSubstitutionDialog() = default;

QString RegexGroupSubstitutionDialog::groupName()
{
  return m_ui->groupNameLineEdit->text();
}

QString RegexGroupSubstitutionDialog::regex()
{
  return m_ui->regexLineEdit->text();
}

QString RegexGroupSubstitutionDialog::substitution()
{
  return m_ui->substitutionLineEdit->text();
}
}
