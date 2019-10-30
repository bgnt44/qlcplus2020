
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDebug>

#include "fixturecalibrationdata.h"


#define PI 3.14159265
#define KXMLQLCFixtureCaliData = "KXMLQLCFixtureCaliData";


FixtureCalibrationData::FixtureCalibrationData():
    pan0(127)
  , panLT(0)
  , panRT(255)
  , panLB(0)
  , panRB(255)
  , tiltLT(0)
  , tiltRT(0)
  , tiltLB(255)
  , tiltRB(255)
  , tilt0(127)
{
   // setName(tr("New Track"));
}

FixtureCalibrationData::~FixtureCalibrationData()
{

}

qint32 FixtureCalibrationData::getPan(qint32 x, qint32 y){

    qint32 xA = getPanLT();
    qint32 xB = getPanRT();
    qint32 xC = getPanLB();
    qint32 xD = getPanRB();

    float xcalc = ((float)(100-y)/100) * (xA + ((xB-xA) * x/100)) + ((float)y/100) * (xC + (xD-xC)*x/100);

   // quint32 xrot = xcalc * (float)cos(3.14159265/180 * ratio) - ycalc * (float)sin(3.14159265/ratio * 180);

    return xcalc;
}

qint32 FixtureCalibrationData::getTilt(qint32 x, qint32 y){

    qint32 yA = getTiltLT();
    qint32 yB = getTiltRT();
    qint32 yC = getTiltLB();
    qint32 yD = getTiltRB();

    float ycalc = ((float)(100-x)/100) * (yA + ((yC-yA) * y/100)) + ((float)x/100) * (yB + (yD-yB)*y/100);

    return ycalc;
}

qint32 FixtureCalibrationData::getMinPan()
{
    int halfpan = (panLT + panRT)/2;
    return halfpan;

}
qint32 FixtureCalibrationData::getMinTilt()
{
    int halftilt = (tiltLT + tiltLB)/2;
    return halftilt;
}
qint32 FixtureCalibrationData::getMaxPan()
{
    int halfpan = (panLB + panRB)/2;
    return halfpan;
}
qint32 FixtureCalibrationData::getMaxTilt()
{
    int halftilt = (tiltRT + tiltRB)/2;
    return halftilt;
}

qint32 FixtureCalibrationData::getPan0(){
    return pan0;
}
qint32 FixtureCalibrationData::getPanLT(){
    return panLT;
}
qint32 FixtureCalibrationData::getPanRT(){
    return panRT;
}
qint32 FixtureCalibrationData::getPanLB(){
    return panLB;
}
qint32 FixtureCalibrationData::getPanRB(){
    return panRB;
}
qint32 FixtureCalibrationData::getTilt0(){
    return tilt0;
}
qint32 FixtureCalibrationData::getTiltLT(){
    return tiltLT;
}
qint32 FixtureCalibrationData::getTiltRT(){
    return tiltRT;
}
qint32 FixtureCalibrationData::getTiltLB(){
    return tiltLB;
}
qint32 FixtureCalibrationData::getTiltRB(){
    return tiltRB;
}

void FixtureCalibrationData::setPoint0(qint32 x,qint32 y){
    pan0 = x;
    tilt0 = y;
}

void FixtureCalibrationData::setPointLT(qint32 x,qint32 y){
    panLT = x;
    tiltLT = y;
}

void FixtureCalibrationData::setPointRT(qint32 x,qint32 y){
    panRT = x;
    tiltRT = y;
}

void FixtureCalibrationData::setPointLB(qint32 x,qint32 y){
    panLB = x;
    tiltLB = y;
}

void FixtureCalibrationData::setPointRB(qint32 x,qint32 y){
    panRB = x;
    tiltRB = y;
}

/****************************************************************************
 * Load & Save
 ****************************************************************************/

bool FixtureCalibrationData::loadXML(QXmlStreamReader &root)
{

    while (root.readNextStartElement())
    {

        if (root.name() == "panRT")
        {
            panRT = (root.readElementText()).toInt();
        }
        else if (root.name() == "panLT")
        {
            panLT = (root.readElementText()).toInt();
        }
        else if (root.name() == "panRB")
        {
            panRB = (root.readElementText()).toInt();
        }
        else if (root.name() == "panLB")
        {
            panLB = (root.readElementText()).toInt();
        }
        else if (root.name() == "tiltLB")
        {
            tiltLB = (root.readElementText()).toInt();
        }
        else if (root.name() == "tiltLT")
        {
            tiltLT = (root.readElementText()).toInt();
        }else if (root.name() == "tiltRB")
        {
            tiltRB = (root.readElementText()).toInt();
        }
        else if (root.name() == "tiltRB")
        {
            tiltRB = (root.readElementText()).toInt();

        }else if (root.name() == "pan0")
        {
            pan0 = (root.readElementText()).toInt();
        }
        else if (root.name() == "tilt0")
        {
            tilt0 = (root.readElementText()).toInt();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown script tag:" << root.name();
            root.skipCurrentElement();
        }

    }

    return true;
}

bool FixtureCalibrationData::saveXML(QXmlStreamWriter *doc)
{
    Q_ASSERT(doc != NULL);

    /* Function tag */
    //doc->writeStartElement("KXMLQLCFixtureCaliData");
    doc->writeTextElement("panLT",QString::number(panLT));
    doc->writeTextElement("panRT",QString::number(panRT));
    doc->writeTextElement("panLB",QString::number(panLB));
    doc->writeTextElement("panRB",QString::number(panRB));
    doc->writeTextElement("tiltLT",QString::number(tiltLT));
    doc->writeTextElement("tiltRT",QString::number(tiltRT));
    doc->writeTextElement("tiltLB",QString::number(tiltLB));
    doc->writeTextElement("tiltRB",QString::number(tiltRB));
    doc->writeTextElement("pan0",QString::number(pan0));
    doc->writeTextElement("tilt0",QString::number(tilt0));

    //doc->writeEndElement();

    /* Common attributes */
    //saveXMLCommon(doc);

    /* Version */
    //doc->writeAttribute(KXMLQLCScriptVersion, QString::number(2));

//    /* Speed */
//    saveXMLSpeed(doc);

//    /* Direction */
//    saveXMLDirection(doc);

//    /* Run order */
//    saveXMLRunOrder(doc);

//    /* Contents */
//    foreach(QString cmd, dataLines())
//    {
//        doc->writeTextElement(KXMLQLCScriptCommand, QUrl::toPercentEncoding(cmd));
//    }

    /* End the <Function> tag */

    return true;
}
