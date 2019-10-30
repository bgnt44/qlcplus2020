/*
  Q Light Controller
  FixtureCalibration.cpp

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

#include <QMessageBox>
#include <QString>
#include <QDebug>
#include <QHash>
#include <QAction>
#include <QDate>

#include "simpledeskengine.h"
#include "fixturecalibrationdata.h"

#include "palettegenerator.h"
#include "fixtureselection.h"
#include "fixturecalibration.h"
#include "virtualconsole.h"
#include "vcsoloframe.h"
#include "vccuelist.h"
#include "rgbmatrix.h"
#include "vcwidget.h"
#include "vcbutton.h"
#include "vcslider.h"
#include "vcframe.h"
#include "fixture.h"
#include "chaser.h"
#include "scene.h"
#include "doc.h"

#include "qlcfixturedef.h"
#include "qlccapability.h"
#include "qlcchannel.h"
#include "vcxypadarea.h"
#include "qlcmacros.h"
#include <math.h>       /* sin */


#define KFixtureColumnName          0
#define KFixtureColumnCaps          1
#define KFixtureColumnManufacturer  2
#define KFixtureColumnModel         3

#define KFunctionName               0
#define KFunctionOddEven            1

#define KWidgetName                 0

FixtureCalibration::FixtureCalibration(QWidget* parent, Doc* doc)
    : QDialog(parent)
    , m_doc(doc)
    , m_engine(new SimpleDeskEngine(doc))
    , m_area(new VCXYPadArea(this))

{
    Q_ASSERT(doc != NULL);
    setupUi(this);
    pan = 0;
    tilt = 0;

    SliderPan->setStyleSheet(CNG_DEFAULT_STYLE);

    sliderTilt->setStyleSheet(CNG_DEFAULT_STYLE);

    QString trbgSS = "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(0, 0, 0, 0), stop:1 rgba(255, 255, 255, 0));";
    m_wizardLogo->setStyleSheet(trbgSS);
    m_introText->setStyleSheet(trbgSS);

    QAction* action = new QAction(this);
    action->setShortcut(QKeySequence(QKeySequence::Close));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(reject()));
    addAction(action);

    m_fixtureTree->sortItems(KFixtureColumnName, Qt::AscendingOrder);
    m_area->setMode(Doc::Operate); // to activate the area
    m_area->setWindowTitle("");
    //m_area->setDegreesRange(degreesRange);
    m_area->setFocus();

    m_gridLayoutArea->addWidget(m_area, 0, 0);

    lst.append(QPointF(50,50));
    lst.append(QPointF(200,50));
    lst.append(QPointF(50,200));
    lst.append(QPointF(200,200));
    lst.append(QPointF(125,125));

    m_area->slotFixturePositionsColor(lst);

    connect(m_area, SIGNAL(positionChanged(QPointF)),
                this, SLOT(getPositionArea(QPointF)));

    sliderTilt->setValue(0);
    SliderPan->setValue(0);

    connect(m_restrictButton,SIGNAL(clicked()),this,SLOT(restrict()));

    connect(m_applyallButton,SIGNAL(clicked()),this,SLOT(applyAll()));

    connect(m_nextButton, SIGNAL(clicked()),
            this, SLOT(slotNextPageClicked()));

    connect(m_tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(slotTabClicked()));

    connect(sliderTilt, SIGNAL(valueChanged(int)),
                this, SLOT(slotTiltSliderValueChanged(int)));

    connect(SliderPan, SIGNAL(valueChanged(int)),
                this, SLOT(slotPanSliderValueChanged(int)));

    connect(m_fixtureTree, SIGNAL(itemSelectionChanged()),
            this,SLOT(slotFixtureTreeChanged()));

    connect(spinBoxPan, SIGNAL(valueChanged(int)),
                        this, SLOT(slotSpinPanChange(int)));

    connect(spinBoxTilt, SIGNAL(valueChanged(int)),
                        this, SLOT(slotSpinTiltChange(int)));

    connect(rbCenter,SIGNAL(toggled(bool)),this,SLOT(getCalibration(bool)));

    connect(rbLeftB,SIGNAL(toggled(bool)),this,SLOT(getCalibration(bool)));

    connect(rbLeftF,SIGNAL(toggled(bool)),this,SLOT(getCalibration(bool)));

    connect(rbRightB,SIGNAL(toggled(bool)),this,SLOT(getCalibration(bool)));

    connect(rbRightF,SIGNAL(toggled(bool)),this,SLOT(getCalibration(bool)));



    checkTabsAndButtons();
}

void FixtureCalibration::getPositionArea(QPointF point)
{
    if(fxi != nullptr){
        SetValue(point.toPoint().x(),point.toPoint().y());
        m_engine->setValue(panChannel, point.toPoint().x());
        m_engine->setValue(tiltChannel, point.toPoint().y());
        setCalibrationData();
    }
}

FixtureCalibration::~FixtureCalibration()
{
    m_paletteList.clear();
}

void FixtureCalibration::slotNextPageClicked()
{
    int newIdx = m_tabWidget->currentIndex() + 1;
    if (newIdx == 4)
        return;

    m_tabWidget->setCurrentIndex(newIdx);
    checkTabsAndButtons();
}

void FixtureCalibration::slotTabClicked()
{
    checkTabsAndButtons();
}

void FixtureCalibration::accept()
{

    GenerateShape();
    m_engine->resetChannel(panChannel);
    m_engine->resetChannel(tiltChannel);
    m_engine->resetUniverse(0);
    m_engine->resetUniverse(1);
    m_engine->clearContents();

    m_doc->setModified();

    QDialog::accept();
}

void FixtureCalibration::reject()
{


    m_engine->resetChannel(panChannel);
    m_engine->resetChannel(tiltChannel);
    m_engine->resetUniverse(0);
    m_engine->resetUniverse(1);
    m_engine->clearContents();

    m_doc->setModified();

    QDialog::reject();
}


void FixtureCalibration::checkTabsAndButtons()
{
    switch(m_tabWidget->currentIndex())
    {
        case 0:
        {
            m_nextButton->setEnabled(true);
            m_tabWidget->setTabEnabled(2, true);
            m_tabWidget->setTabEnabled(3, false);
        }
        break;
        case 1:
        {

            m_nextButton->setEnabled(true);
            m_tabWidget->setTabEnabled(2, true);

        }
        break;
        case 2:
        {
            LoadFixtureCalibrationDate();
            m_tabWidget->setTabEnabled(3, true);
            m_nextButton->setEnabled(true);

        }
        break;
        case 3:
            m_nextButton->setEnabled(false);
        break;
    }
}

/****************************************************************************
 * Fixtures
 ****************************************************************************/

QTreeWidgetItem *FixtureCalibration::getFixtureGroupItem(QString manufacturer, QString model)
{
    for (int i = 0; i < m_fixtureTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem *item = m_fixtureTree->topLevelItem(i);
        if (item->text(KFixtureColumnManufacturer) == manufacturer &&
            item->text(KFixtureColumnModel) == model)
                return item;
    }
    // if we're here then the group doesn't exist. Create it
    QTreeWidgetItem* newGrp = new QTreeWidgetItem(m_fixtureTree);
    newGrp->setText(KFixtureColumnName, tr("%1 group").arg(model));
    newGrp->setIcon(KFixtureColumnName, QIcon(":/group.png"));
    newGrp->setText(KFixtureColumnManufacturer, manufacturer);
    newGrp->setText(KFixtureColumnModel, model);
    newGrp->setExpanded(true);
    return newGrp;
}

void FixtureCalibration::addFixture(quint32 fxi_id)
{
    Fixture* fxi = m_doc->fixture(fxi_id);
    Q_ASSERT(fxi != NULL);


    QTreeWidgetItem *groupItem = getFixtureGroupItem(fxi->fixtureDef()->manufacturer(), fxi->fixtureDef()->model());
    Q_ASSERT(groupItem != NULL);

    QTreeWidgetItem* item = new QTreeWidgetItem(groupItem);
    item->setText(KFixtureColumnName, fxi->name());
    item->setIcon(KFixtureColumnName, fxi->getIconFromType());
    item->setData(KFixtureColumnName, Qt::UserRole, fxi_id);

    m_fixtureTree->resizeColumnToContents(KFixtureColumnName);
}

void FixtureCalibration::slotAddClicked()
{
    FixtureSelection fs(this, m_doc);
    fs.setMultiSelection(true);
    fs.setDisabledFixtures(fixtureIds());
    if (fs.exec() == QDialog::Accepted)
    {
        QListIterator <quint32> it(fs.selection());
        while (it.hasNext() == true)
            addFixture(it.next());


    }
    checkTabsAndButtons();
}

void FixtureCalibration::slotRemoveClicked()
{
    QListIterator <QTreeWidgetItem*> it(m_fixtureTree->selectedItems());
    while (it.hasNext() == true)
        delete it.next();

    checkTabsAndButtons();
}

QList <quint32> FixtureCalibration::fixtureIds() const
{
    QList <quint32> list;
    for (int i = 0; i < m_fixtureTree->topLevelItemCount(); i++)
    {
        QTreeWidgetItem* item(m_fixtureTree->topLevelItem(i));
        Q_ASSERT(item != NULL);

        for (int j = 0; j < item->childCount(); j++)
        {
            QTreeWidgetItem *child = item->child(j);
            Q_ASSERT(child != NULL);

            list << child->data(KFixtureColumnName, Qt::UserRole).toInt();
        }
    }

    return list;
}

FixtureCalibrationData* getCalibData(Fixture* fxi)
{
    FixtureCalibrationData* dat = fxi->getCalibrationData();
    if(dat == nullptr)
    {
        fxi->AddCalibrationData();
        dat = fxi->getCalibrationData();;
    }
    return dat;
}

void FixtureCalibration::slotFixtureTreeChanged()
{
    sliderTilt->setEnabled(false);
    SliderPan->setEnabled(false);
    grpPosition->setEnabled(false);

    if(m_fixtureTree->selectedItems().length() == 1)
    {
        QTreeWidgetItem *fxItem =  m_fixtureTree->selectedItems()[0];
        if(fxItem != nullptr)
        {
            if(fxItem->childCount() == 0)
            {
                quint32 fxID = fxItem->data(KFixtureColumnName, Qt::UserRole).toUInt();

                fxi = m_doc->fixture(fxID);
                Q_ASSERT(fxi != NULL);

                lblNomFixture->setText(fxi->name());
                sliderTilt->setEnabled(true);
                SliderPan->setEnabled(true);
                grpPosition->setEnabled(true);

                panChannel = fxi->universeAddress() + fxi->channelNumber(QLCChannel::Pan,QLCChannel::MSB);
                tiltChannel = fxi->universeAddress() + fxi->channelNumber(QLCChannel::Tilt,QLCChannel::MSB);

                int panVal = m_engine->value(panChannel);
                int tiltVal = m_engine->value(tiltChannel);

                SetValue(panVal,tiltVal);
                getCalibration(false);

                FixtureCalibrationData* dat = getCalibData(fxi);

                lst[0] = QPoint(dat->getPanLT(),dat->getTiltLT());
                lst[1] = QPoint(dat->getPanRT(),dat->getTiltRT());
                lst[2] = QPoint(dat->getPanLB(),dat->getTiltLB());
                lst[3] = QPoint(dat->getPanRB(),dat->getTiltRB());
                lst[4] = QPoint(dat->getPan0(),dat->getTilt0());

                m_area->slotFixturePositionsColor(lst);

                rbLeftF->setChecked(true);
            }
        }
    }
}
/********************************************************************
 * Functions
 ********************************************************************/
void FixtureCalibration::LoadFixtureCalibrationDate()
{
//    QTreeWidgetItem *fxGrpItem = m_fixtureTree->topLevelItem(0);
//    quint32 fxID = fxItem->data(KFixtureColumnName, Qt::UserRole).toUInt();
//    Fixture* fxi = m_doc->fixture(fxID);
//    Q_ASSERT(fxi != NULL);
//    panChannel = fxi->universeAddress() + fxi->channelNumber(QLCChannel::Pan,QLCChannel::MSB);
//    tiltChannel = fxi->universeAddress() + fxi->channelNumber(QLCChannel::Tilt,QLCChannel::MSB);
}

void FixtureCalibration::slotSpinPanChange(int value)
{
    slotPanSliderValueChanged(value);
}

void FixtureCalibration::slotSpinTiltChange(int value)
{
    slotTiltSliderValueChanged(value);
}

void FixtureCalibration::slotPanSliderValueChanged(int value)
{
    if(panChannel > -1){

        spinBoxPan->blockSignals(true);
        spinBoxPan->setValue(value);
        spinBoxPan->blockSignals(false);

        m_engine->setValue(panChannel, value);
        pan = value;

        setCalibrationData();
    }
}

void FixtureCalibration::slotTiltSliderValueChanged(int value)
{
     if(tiltChannel > -1)
     {
      spinBoxTilt->blockSignals(true);
      spinBoxTilt->setValue(value);
      spinBoxTilt->blockSignals(false);

      m_engine->setValue(tiltChannel, value);
      tilt = value;

      setCalibrationData();
     }
}

void FixtureCalibration::SetValue(qint32 valPan, qint32 valTilt)
{
    pan = valPan;
    tilt = valTilt;

    spinBoxPan->blockSignals(true);
    spinBoxTilt->blockSignals(true);
    sliderTilt->blockSignals(true);
    SliderPan->blockSignals(true);

    sliderTilt->setValue(tilt);
    SliderPan->setValue(pan);
    spinBoxPan->setValue(pan);
    spinBoxTilt->setValue(tilt);

    spinBoxPan->blockSignals(false);
    spinBoxTilt->blockSignals(false);
    sliderTilt->blockSignals(false);
    SliderPan->blockSignals(false);

}

void FixtureCalibration::getCalibration(bool val)
{
    Q_UNUSED(val);
    if(fxi != NULL)
    {
        FixtureCalibrationData* data =  getCalibData(fxi);
        if(rbLeftF->isChecked() == true)
        {
            SetValue(data->getPanLT(),data->getTiltLT());
        }
        if(rbRightF->isChecked() == true)
        {
            SetValue(data->getPanRT(),data->getTiltRT());
        }
        if(rbLeftB->isChecked() == true)
        {
             SetValue(data->getPanLB(),data->getTiltLB());
        }
        if(rbRightB->isChecked() == true)
        {
             SetValue(data->getPanRB(),data->getTiltRB());
        }
        if(rbCenter->isChecked() == true)
        {
             SetValue(data->getPan0(),data->getTilt0());
        }
    }
}

void FixtureCalibration::setCalibrationData()
{
    if(fxi != NULL)
    {
        FixtureCalibrationData* data =  getCalibData(fxi);
        if(rbLeftF->isChecked() == true)
        {
            data->setPointLT(pan,tilt);
            lst[0] = QPoint(pan,tilt);
        }
        if(rbRightF->isChecked() == true)
        {
            data->setPointRT(pan,tilt);
            lst[1] = QPoint(pan,tilt);
        }
        if(rbLeftB->isChecked() == true)
        {
            data->setPointLB(pan,tilt);
            lst[2] = QPoint(pan,tilt);
        }
        if(rbRightB->isChecked() == true)
        {
            data->setPointRB(pan,tilt);
            lst[3] = QPoint(pan,tilt);
        }
        if(rbCenter->isChecked() == true)
        {
            data->setPoint0(pan,tilt);
            lst[4] = QPoint(pan,tilt);
        }

        m_area->slotFixturePositionsColor(lst);
    }
}

Scene* FixtureCalibration::CreateScene(quint32 x, quint32 y, quint32 angle, quint32 current, QString name)
{

    QString groupName;
    for(int i=0; i<fixtureIds().length(); i++)
    {
        groupName += QString::number(fixtureIds()[i]);
    }
    QDateTime dateTimeC = QDateTime::currentDateTime();
    groupName += "_" + QString::number(dateTimeC.toSecsSinceEpoch());

    Scene* scene = new Scene(m_doc);
    scene->setPath("AutoCal/"+groupName+"/Moves");
    scene->setName(name);
    //quint32 compteur = fixtureIds().count();
    //quint32 current = 0;
    foreach (quint32 fx, fixtureIds()) {

        Fixture* fix = m_doc->fixture(fx);
        FixtureCalibrationData* data =  getCalibData(fix);
        int panC = fix->channelNumber(QLCChannel::Pan,QLCChannel::MSB);
        int tiltC = fix->channelNumber(QLCChannel::Tilt,QLCChannel::MSB);

        float ratio = (float)current * angle;

        if(ratio == 0)
            ratio = 360;

        float realAngle = 3.14159265358979323846 * ratio/180;
        float originX = ((float)x - 50);
        float originY = ((float)y - 50);


        int newX = 50 + originX * cos(realAngle) - originY * sin(realAngle) ;
        int newY = 50 + originX * sin(realAngle) + originY * cos(realAngle);

        scene->setValue(fx,panC,data->getPan(newX,newY));

        scene->setValue(fx,tiltC,data->getTilt(newX,newY));

        current++;
    }
    m_doc->addFunction(scene);
    return scene;
}

void FixtureCalibration::GenerateChaser(QString name, QList<Scene*> scenes)
{
    QString groupName;
    for(int i=0; i<fixtureIds().length(); i++)
    {
        groupName += QString::number(fixtureIds()[i]);
    }
    QDateTime current = QDateTime::currentDateTime();
    groupName += "_" + QString::number(current.toSecsSinceEpoch());
    Chaser* chaser = new Chaser(m_doc);
    chaser->setName(name);
    chaser->setPath("AutoCal/"+groupName+"/Slow");
    chaser->setDurationMode(Chaser::PerStep);
    chaser->setFadeInMode(Chaser::PerStep);
    chaser->setFadeOutMode(Chaser::PerStep);

    foreach(Scene* scene, scenes)
    {
        ChaserStep step(scene->id(),1500,0,0);
        chaser->addStep(step);
    }

    m_doc->addFunction(chaser);

    Chaser* chaser2 = new Chaser(m_doc);
    chaser2->setName(name);
    chaser2->setPath("AutoCal/"+groupName+"/Fast");
    chaser2->setDurationMode(Chaser::PerStep);
    chaser2->setFadeInMode(Chaser::PerStep);
    chaser2->setFadeOutMode(Chaser::PerStep);

    foreach(Scene* scene, scenes)
    {
        ChaserStep step(scene->id(),600,0,0);
        chaser2->addStep(step);
    }

    m_doc->addFunction(chaser2);

    Chaser* chaserRpt = new Chaser(m_doc);
    chaserRpt->setName(name);
    chaserRpt->setPath("AutoCal/"+groupName+"/Rpt");
    chaserRpt->setDurationMode(Chaser::PerStep);
    chaserRpt->setFadeInMode(Chaser::PerStep);
    chaserRpt->setFadeOutMode(Chaser::PerStep);


    ChaserStep step(chaser2->id(),10000,0,0);
    chaserRpt->addStep(step);


    m_doc->addFunction(chaserRpt);
}

void FixtureCalibration::applyAll()
{
    if(fxi != nullptr)
    {
        FixtureCalibrationData* dataSource =  getCalibData(fxi);
        foreach (quint32 fx, fixtureIds()) {
            if(fxi->id() != fx){
                Fixture* fix = m_doc->fixture(fx);
                FixtureCalibrationData* dataDest =  getCalibData(fix);
                dataDest->setPointLB(dataSource->getPanLB(),dataSource->getTiltLB());
                dataDest->setPointLT(dataSource->getPanLT(),dataSource->getTiltLT());
                dataDest->setPointRB(dataSource->getPanRB(),dataSource->getTiltRB());
                dataDest->setPointRT(dataSource->getPanRT(),dataSource->getTiltRT());
            }
        }
    }
}


void FixtureCalibration::GenerateShape()
{

     qint32 compteur = fixtureIds().count();
     if(compteur > 0)
     {
         Scene* sLeftRight = CreateScene(0,50,180,0,"left_right");
         Scene* sLeftRight2 = CreateScene(0,50,180,1,"left_right2");

         Scene* sTopBot = CreateScene(50,0,180,0,"top_bottom");
         Scene* sTopBot2 = CreateScene(50,0,180,1,"top_bottom2");

         Scene* sCross = CreateScene(50,0,90,0,"crossing");
         Scene* sCross2 = CreateScene(50,0,90,1,"crossing2");
         Scene* sCross3 = CreateScene(50,0,90,2,"crossing3");
         Scene* sCross4 = CreateScene(50,0,90,3,"crossing4");

         Scene* smCross = CreateScene(25,50,90,0,"mini_crossing");
         Scene* smCross2 = CreateScene(25,50,90,1,"mini_crossing2");
         Scene* smCross3 = CreateScene(25,50,90,2,"mini_crossing3");
         Scene* smCross4 = CreateScene(25,50,90,3,"mini_crossing4");

         Scene* center = CreateScene(50,50,0,0,"center");

         Scene* intermediate = CreateScene(25,25,90,0,"intermediate");
         Scene* intermediate2 = CreateScene(25,25,90,1,"intermediate2");
         Scene* intermediate3 = CreateScene(25,25,90,2,"intermediate3");
         Scene* intermediate4 = CreateScene(25,25,90,3,"intermediate4");
         Scene* intermediateAll1 = CreateScene(25,25,0,0,"intermediateAll1");
         Scene* intermediateAll2 = CreateScene(75,25,0,0,"intermediateAll2");
         Scene* intermediateAll3 = CreateScene(75,75,0,0,"intermediateAll3");
         Scene* intermediateAll4 = CreateScene(25,75,0,0,"intermediateAll4");

         if(compteur > 4)
         {
             Scene* inter8_1 = CreateScene(25,25,45,0,"inter8_1");
             Scene* inter8_2 = CreateScene(25,25,45,1,"inter8_2");
             Scene* inter8_3 = CreateScene(25,25,45,2,"inter8_3");
             Scene* inter8_4 = CreateScene(25,25,45,3,"inter8_4");
             Scene* inter8_5 = CreateScene(25,25,45,4,"inter8_5");
             Scene* inter8_6 = CreateScene(25,25,45,5,"inter8_6");
             Scene* inter8_7 = CreateScene(25,25,45,6,"inter8_7");
             Scene* inter8_8 = CreateScene(25,25,45,7,"inter8_8");
             GenerateChaser("inter8",{inter8_1,inter8_2,inter8_3,inter8_4,inter8_5,inter8_6,inter8_7,inter8_8});

         }

         Scene* smallMove = CreateScene(40,40,90,0,"smallMove");
         Scene* bigmove = CreateScene(3,3,90,3,"smallMove");

         GenerateChaser("Left_Right",{sLeftRight,sLeftRight2});
         GenerateChaser("Top_Bottom",{sTopBot,sTopBot2});
         GenerateChaser("Crossing",{sCross,sCross2,sCross3,sCross4});
         GenerateChaser("Small_Crossing",{smCross,smCross2,smCross3,smCross4});
         GenerateChaser("intermediate",{intermediate,intermediate2,intermediate3,intermediate4});
         GenerateChaser("intermediate_Packed",{intermediate,intermediateAll2,intermediate3,intermediateAll4});
         GenerateChaser("intermediate_Packed2",{intermediate,intermediateAll3,intermediate3,intermediateAll1});

         GenerateChaser("Random_Move",{center,smCross,sCross4,intermediateAll2,sLeftRight,sTopBot2});
         GenerateChaser("Flower",{center,smallMove,intermediate3,bigmove,intermediate4,smallMove});

     }

}
void FixtureCalibration::restrict()
{
    if(fxi != nullptr)
    {
        FixtureCalibrationData* dataSource =  getCalibData(fxi);
        // manage left

        qint32 minleft =(dataSource->getPanLB() < dataSource->getPanLT())? dataSource->getPanLT():dataSource->getPanLB();
        qint32 maxright=(dataSource->getPanRB() > dataSource->getPanRT())?dataSource->getPanRT():dataSource->getPanRB();
        qint32 mintop=(dataSource->getTiltLT() > dataSource->getTiltRT())?dataSource->getTiltLT() :dataSource->getTiltRT();
        qint32 maxbtm=(dataSource->getTiltLB() < dataSource->getTiltRB())?dataSource->getTiltLB():dataSource->getTiltRB();

        if(dataSource->getPanLB() < minleft)
        {
            dataSource->setPointLB(minleft,dataSource->getTiltLB());
        }
        if(dataSource->getPanLB() > maxright)
        {
            dataSource->setPointLB(maxright,dataSource->getTiltLB());
        }
        if(dataSource->getTiltLB() < mintop)
        {
            dataSource->setPointLB(dataSource->getPanLB(),mintop);
        }
        if(dataSource->getTiltLB() > maxbtm)
        {
            dataSource->setPointLB(dataSource->getPanLB(),maxbtm);
        }

        if(dataSource->getPanRB() < minleft)
        {
            dataSource->setPointRB(minleft,dataSource->getTiltRB());
        }
        if(dataSource->getPanRB() > maxright)
        {
            dataSource->setPointRB(maxright,dataSource->getTiltRB());
        }
        if(dataSource->getTiltRB() < mintop)
        {
            dataSource->setPointRB(dataSource->getPanRB(),mintop);
        }
        if(dataSource->getTiltRB() > maxbtm)
        {
            dataSource->setPointRB(dataSource->getPanRB(),maxbtm);
        }

        if(dataSource->getPanRT() < minleft)
        {
            dataSource->setPointRT(minleft,dataSource->getTiltRT());
        }
        if(dataSource->getPanRT() > maxright)
        {
            dataSource->setPointRT(maxright,dataSource->getTiltRT());
        }
        if(dataSource->getTiltRT() < mintop)
        {
            dataSource->setPointRT(dataSource->getPanRT(),mintop);
        }
        if(dataSource->getTiltRT() > maxbtm)
        {
            dataSource->setPointRT(dataSource->getPanRT(),maxbtm);
        }

        if(dataSource->getPanLT() < minleft)
        {
            dataSource->setPointLT(minleft,dataSource->getTiltLT());
        }
        if(dataSource->getPanLT() > maxright)
        {
            dataSource->setPointLT(maxright,dataSource->getTiltLT());
        }
        if(dataSource->getTiltLT() < mintop)
        {
            dataSource->setPointLT(dataSource->getPanLT(),mintop);
        }
        if(dataSource->getTiltLT() > maxbtm)
        {
            dataSource->setPointLT(dataSource->getPanLT(),maxbtm);
        }

        lst[0] = QPoint(dataSource->getPanLT(),dataSource->getTiltLT());
        lst[1] = QPoint(dataSource->getPanRT(),dataSource->getTiltRT());
        lst[2] = QPoint(dataSource->getPanLB(),dataSource->getTiltLB());
        lst[3] = QPoint(dataSource->getPanRB(),dataSource->getTiltRB());

        m_area->slotFixturePositionsColor(lst);
    }
}
