#ifndef FIXTURECALIBRATIONDATA_H
#define FIXTURECALIBRATIONDATA_H

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QObject>

class QString;
class Doc;

class FixtureCalibrationData
{

public:
    FixtureCalibrationData();
    ~FixtureCalibrationData();

    void setPoint0(qint32 x,qint32 y);
    void setPointLT(qint32 x,qint32 y);
    void setPointRT(qint32 x,qint32 y);
    void setPointLB(qint32 x,qint32 y);
    void setPointRB(qint32 x,qint32 y);

    bool saveXML(QXmlStreamWriter *doc);
    bool loadXML(QXmlStreamReader &root);

    qint32 getPan0();
    qint32 getPanLT();
    qint32 getPanRT();
    qint32 getPanLB();
    qint32 getPanRB();
    qint32 getTiltLT();
    qint32 getTiltRT();
    qint32 getTiltLB();
    qint32 getTiltRB();
    qint32 getTilt0();

    qint32 getPan(qint32 x, qint32 y);
    qint32 getTilt(qint32 x, qint32 y);

    qint32 getMinPan();
    qint32 getMinTilt();
    qint32 getMaxPan();
    qint32 getMaxTilt();


public:
    qint32 pan0;
    qint32 panLT;
    qint32 panRT;
    qint32 panLB;
    qint32 panRB;
    qint32 tiltLT;
    qint32 tiltRT;
    qint32 tiltLB;
    qint32 tiltRB;
    qint32 tilt0;
public slots:
};

#endif // FIXTURECALIBRATION_H
