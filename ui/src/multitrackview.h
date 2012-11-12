/*
  Q Light Controller
  multitrackview.h

  Copyright (C) Massimo Callegari

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef MULTITRACKVIEW_H
#define MULTITRACKVIEW_H

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QSlider>
#include <QWidget>

#include "sceneitems.h"
#include "chaser.h"
#include "track.h"

class MultiTrackView : public QGraphicsView
{
    Q_OBJECT
    
public:
    MultiTrackView(QWidget *parent = 0);

    /** Update the multitrack view with the scene elements */
    void resetView();

    /** Add a new track to the view */
    void addTrack(Track *track);

    /** Add a new sequence to the given track */
    void addSequence(Chaser *chaser);

    /** Delete the currently selected sequence */
    quint32 deleteSelectedSequence();

    /** Move cursor to a given time */
    void moveCursor(quint32 timePos);

    /** Reset cursor to initial position */
    void rewindCursor();

    void activateTrack(Track *track);

    /** get the selected sequence item. If none, returns NULL */
    SequenceItem *getSelectedSequence();

private:
    /** Get time of current cursor position */
    quint32 getTimeFromPosition();

    /** Return position in pixel of a given time (in msec) */
    quint32 getPositionFromTime(quint32 time);

    /** Get the index of the currently selected track */
    int getActiveTrack();

private:
    QGraphicsScene *m_scene;
    QSlider *m_timeSlider;
    SceneHeaderItem *m_header;
    SceneCursorItem *m_cursor;
    QList <TrackItem *> m_tracks;
    QList <SequenceItem *> m_sequences;

signals:
    void sequenceMoved(SequenceItem *item);
    void viewClicked(QMouseEvent * e);
    void timeChanged(quint32 msec);
    void trackClicked(Track *track);

public slots:
    void mouseReleaseEvent(QMouseEvent * e);

protected slots:
    void slotMoveCursor(QGraphicsSceneMouseEvent *event);
    void slotTimeScaleChanged(int val);
    void slotTrackClicked(TrackItem*);
    void slotTrackSoloFlagChanged(TrackItem*, bool);
    void slotTrackMuteFlagChanged(TrackItem*, bool);

    void slotSequenceMoved(QGraphicsSceneMouseEvent *, SequenceItem *);
};

#endif
