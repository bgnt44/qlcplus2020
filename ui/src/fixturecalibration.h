/*
  Q Light Controller
  functionwizard.h

  Copyright (C) Heikki Junnila

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#ifndef FIXTURECALIBRATION_H
#define FIXTURECALIBRATION_H

#include <QDialog>
#include <QList>

#include "ui_fixturecalibration.h"
#include "palettegenerator.h"
#include "scenevalue.h"
#include "function.h"
#include "vcxypadarea.h"
#include "clickandgoslider.h"

class SimpleDeskEngine;
class QLCChannel;
class VCWidget;
class Fixture;
class Scene;
class Doc;

/** @addtogroup ui_functions
 * @{
 */

class FixtureCalibration : public QDialog, public Ui_FixtureCalibration
{
    Q_OBJECT

public:
    FixtureCalibration(QWidget* parent, Doc* doc);
    ~FixtureCalibration();

protected slots:
    void slotNextPageClicked();
    void slotTabClicked();
    void accept();
    void reject();
    void slotPanSliderValueChanged(int value);
    void slotTiltSliderValueChanged(int value);
    void slotSpinPanChange(int value);
    void slotSpinTiltChange(int value);

private:
    void checkTabsAndButtons();

private:
    Doc* m_doc;
    int panChannel = -1;
    int tiltChannel = -1;
    SimpleDeskEngine *m_engine;
    Fixture* fxi;
    qint32 pan;
    qint32 tilt;
    /********************************************************************
     * Fixtures
     ********************************************************************/
protected:
    /** Create or retrieve an existing item to group fixtures of the same type */
    QTreeWidgetItem *getFixtureGroupItem(QString manufacturer, QString model);

    /** Add a fixture to the tree widget */
    void addFixture(quint32 fxi_id);

    /** Get a list of currently selected fixture ids */
    QList <quint32> fixtureIds() const;
    void SetValue(qint32 pan, qint32 tilt);
    ClickAndGoSlider* cSliderPan;
    ClickAndGoSlider* cSliderTilt;
    VCXYPadArea* m_area;
    QVariantList lst;
    void GenerateChaser(QString name, QList<Scene*> scenes);
protected slots:
    void slotAddClicked();
    void slotRemoveClicked();
    void slotFixtureTreeChanged();
    void getCalibration(bool val);
    void setCalibrationData();
    void getPositionArea(QPointF point);
    void applyAll();
    void restrict();
    /********************************************************************
     * Functions
     ********************************************************************/
protected:

    void addFunctionsGroup(QTreeWidgetItem *fxGrpItem, QTreeWidgetItem *grpItem,
                           QString name, PaletteGenerator::PaletteType type);

    /** Populate the available functions tree based on the available fixtures */
    //void updateAvailableFunctionsTree();

    /** Create or retrieve an existing item to group functions of the same type */
    QTreeWidgetItem *getFunctionGroupItem(const Function *func);

    /** Populate the result functions tree based on selected preset functions */
    void updateResultFunctionsTree();

    void LoadFixtureCalibrationDate();
    void GenerateShape();
    Scene* CreateScene(quint32 x, quint32 y,quint32 angle, quint32 current,QString name);


protected slots:
   // void slotFunctionItemChanged(QTreeWidgetItem* item, int col);

protected:
    QList<PaletteGenerator *> m_paletteList;


};

/** @} */

#endif
