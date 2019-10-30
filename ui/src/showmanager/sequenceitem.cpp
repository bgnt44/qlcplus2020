/*
  Q Light Controller Plus
  sequenceitem.cpp

  Copyright (C) Massimo Callegari

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

#include <QApplication>
#include <QPainter>
#include <QMenu>
#include <QPixmapCache>

#include "sequenceitem.h"
#include "headeritems.h"
#include "chaserstep.h"
#include "trackitem.h"

SequenceItem::SequenceItem(Chaser *seq, ShowFunction *func)
    : ShowItem(func)
    , m_chaser(seq)
    , m_selectedStep(-1)
{
    Q_ASSERT(seq != NULL);

    if (func->color().isValid())
        setColor(func->color());
    else
        setColor(ShowFunction::defaultColor(Function::ChaserType));

    if (func->duration() == 0)
        func->setDuration(seq->totalDuration());

    calculateWidth();

    connect(m_chaser, SIGNAL(changed(quint32)),
            this, SLOT(slotSequenceChanged(quint32)));
}

void SequenceItem::calculateWidth()
{
    int newWidth = 0;
    unsigned long seq_duration = m_chaser->totalDuration();

    if (seq_duration != 0)
        newWidth = ((50/(float)getTimeScale()) * (float)seq_duration) / 1000;

    if (newWidth < (50 / m_timeScale))
        newWidth = 50 / m_timeScale;
    setWidth(newWidth);
}

QPixmap SequenceItem::generatePixmap(Chaser* chaser, float xpos, float maxTime)
{

       QPixmap pixmap(getWidth(), 25);
       QPainter painter(&pixmap);
       painter.setBackgroundMode(Qt::TransparentMode);
       painter.setBrush(Qt::darkGray);
       painter.drawRect(0,0,getWidth(),25);
       paintChaser(&painter,chaser,xpos,maxTime);

       return pixmap;
}

float SequenceItem::paintChaser(QPainter *painter, Chaser* chaser, float xpos, float maxTime)
{
    //Q_UNUSED(maxTime);
    float timeScale = 50/(float)m_timeScale;
    int stepIdx = 0;
    if (this->isSelected() == false)
        m_selectedStep = -1;
    float indent = 0;
    float indentSt = 0;
    if(false)
    {
        foreach (ChaserStep step, chaser->steps())
        {

            uint stepFadeIn = step.fadeIn;
            uint stepFadeOut = step.fadeOut;
            uint stepDuration = step.duration;


            if (chaser->fadeInMode() == Chaser::Common)
                stepFadeIn = chaser->fadeInSpeed();
            if (chaser->fadeOutMode() == Chaser::Common)
                stepFadeOut = m_chaser->fadeOutSpeed();
            if (chaser->durationMode() == Chaser::Common)
                stepDuration = chaser->duration();

            float stepWidth = ((timeScale * (float)stepDuration) / 1000);

            if(maxTime == 0 || xpos <= maxTime)
            {

                Function* stepFunc = step.resolveFunction(chaser->doc());

                if(stepFunc->type() == Function::ChaserType)
                {
                    Chaser* child = qobject_cast<Chaser*>(stepFunc);
                    if(child->runOrder() == Chaser::Loop)
                    {
                        float limit = xpos + stepWidth;
                        while(xpos < limit)
                        {
                            xpos = paintChaser(painter, child, xpos, limit);
                        }
                    }
                    else
                    {
                        paintChaser(painter, child, xpos, maxTime);
                    }
                }

                if(stepFunc->type() == Function::SceneType)
                {
                    Scene* childScene = qobject_cast<Scene*>(stepFunc);
                    int nombreCarres = (stepWidth / 20.0);
                    int courant = 0;
                    foreach(SceneValue val, childScene->values())
                    {
                        if(courant < nombreCarres)
                        {
                            courant = courant + 1;
                            uchar actualValue = val.value;
                            foreach(SceneValue sv, step.values)
                            {
                                if(sv.fxi == val.fxi && sv.channel == val.channel)
                                {
                                    actualValue = sv.value;
                                }
                            }
                        //Eviter de dessiner les infos sur les petites largeurs

                            QList<QLCCapability*> caps = m_chaser->doc()->fixture(val.fxi)->channel(val.channel)->capabilities();
                            foreach (QLCCapability* cap, caps)
                            {
                                if(actualValue >= cap->min() && actualValue < cap->max())
                                {
                                    if (cap->presetType() == QLCCapability::Picture)
                                    {
                                        QString icon = cap->resource(0).toString();
                                        painter->setPen(Qt::white);
                                        painter->drawRect(5 + indent, 2, 20, 20);
                                        painter->drawPixmap(5 + indent, 2, 20, 20, QIcon(icon).pixmap(20, 20));
                                        indent = indent + 22;

                                    }
                                    else if (cap->presetType() == QLCCapability::SingleColor)
                                    {
                                       QColor col1 = cap->resource(0).value<QColor>();
                                       painter->setPen(Qt::white);
                                       painter->setBrush(QBrush(col1));
                                       painter->drawRect(5 + indent, 2, 20, 20);
                                       indent = indent + 22;

                                    }
                                    else if (cap->presetType() == QLCCapability::DoubleColor)
                                    {
                                       QColor col1 = cap->resource(0).value<QColor>();
                                       QColor col2 = cap->resource(1).value<QColor>();
                                       painter->setPen(Qt::white);
                                       painter->setBrush(QBrush(col1));

                                       painter->drawRect(5 + indent, 2, 10, 20);
                                       painter->setBrush(QBrush(col2));
                                       painter->drawRect(15 + indent, 2, 10, 20);
                                       indent = indent + 22;

                                    } else
                                    {
                                       QFont fontOld = painter->font() ;
                                       QFont font;
                                       font.setPixelSize(7);
                                       painter->setBrush(QBrush(Qt::white));
                                       painter->setPen(Qt::white);
                                       painter->setFont(font);
                                       painter->drawRect(5 + indent, 2, 20, 20);
                                       painter->setPen(Qt::black);

                                       painter->drawText(6 + indent,4,19,19, Qt::AlignLeft | Qt::TextWordWrap | Qt::AlignTop,cap->name());
                                       indent = indent + 22;
                                       painter->setFont(fontOld);

                                    }

                                }
                            }
                            caps.length();
                        }
                    }

                }
                // draw fade in line
                if (stepFadeIn > 0)
                {
                    int fadeXpos = xpos + ((timeScale * (float)stepFadeIn) / 1000);
                    // doesn't even draw it if too small
                    if (fadeXpos - xpos > 5)
                    {
                        painter->setPen(QPen(Qt::gray, 1));
                        painter->drawLine(xpos, getHeight(), fadeXpos, getHeight()/2);
                        painter->drawLine(fadeXpos, getHeight()/2, fadeXpos+20, getHeight()/2);
                    }
                }


                // draw selected step
                if (stepIdx == m_selectedStep)
                {
                    painter->setPen(QPen(Qt::green, 3));
                    painter->setBrush(QBrush(Qt::NoBrush));
                    painter->drawRect(xpos, 0, stepWidth, getHeight() - 3);
                }
                xpos += stepWidth;
                indentSt = xpos;
                indent = indentSt;

                // draw step vertical delimiter
                painter->setPen(QPen(Qt::white, 1));
                painter->drawLine(xpos, 1, xpos, getHeight() - 4);

                // draw fade out line
                if (stepFadeOut > 0)
                {
                    int fadeXpos = xpos + ((timeScale * (float)stepFadeOut) / 1000);
                    // doesn't even draw it if too small
                    if (fadeXpos - xpos > 5)
                    {
                        painter->setPen(QPen(Qt::gray, 1));
                        painter->drawLine(xpos, getHeight()/2, fadeXpos, getHeight());
                    }
                }

                stepIdx++;
            }
        }
    }
    return xpos;
}

void SequenceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{

    ShowItem::paint(painter, option, widget);
    //float stepWidth = ((timeScale * (float)m_chaser->duration()) / 1000);
    //paintChaser(painter, m_chaser,0,0);
    QPixmap pm;
    if (!QPixmapCache::find(m_chaser->name() + QString::number(m_chaser->id()), &pm)) {
        pm = generatePixmap(m_chaser, 0, 0);
        QPixmapCache::insert(m_chaser->name()+ QString::number(m_chaser->id()), pm);
    }
    QRegion r(QRect(0, 0, getWidth(),getHeight()));
    painter->setClipRegion(r);

    painter->drawPixmap(1, 1, pm);
    ShowItem::postPaint(painter);
}

void SequenceItem::setTimeScale(int val)
{
    ShowItem::setTimeScale(val);
    calculateWidth();
}

void SequenceItem::setDuration(quint32 msec, bool stretch)
{
    if(stretch)
    {
        m_chaser->setTotalDuration(msec);
    }
    else
    {
        m_chaser->setLastStepDuration(msec);
    }
}

QString SequenceItem::functionName()
{
    if (m_chaser)
        return m_chaser->name();
    return QString();
}

void SequenceItem::setSelectedStep(int idx)
{
    m_selectedStep = idx;
    update();
}

Chaser *SequenceItem::getChaser()
{
    return m_chaser;
}

void SequenceItem::slotSequenceChanged(quint32)
{
    prepareGeometryChange();
    calculateWidth();
    if (m_function)
        m_function->setDuration(m_chaser->totalDuration());
    updateTooltip();

    QPixmap pm = generatePixmap(m_chaser, 0, 0);
    QPixmapCache::insert(m_chaser->name() + QString::number(m_chaser->id()), pm);
    update();
}

void SequenceItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *)
{
    QMenu menu;
    QFont menuFont = qApp->font();
    menuFont.setPixelSize(14);
    menu.setFont(menuFont);

    foreach(QAction *action, getDefaultActions())
        menu.addAction(action);

    menu.exec(QCursor::pos());
}
