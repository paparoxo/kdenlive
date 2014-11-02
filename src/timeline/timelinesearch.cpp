/*
Copyright (C) 2010  Jean-Baptiste Mardelle <jb@kdenlive.org>
Copyright (C) 2014  Till Theato <root@ttill.de>
This file is part of kdenlive. See www.kdenlive.org.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "timelinesearch.h"
#include "core.h"
#include "mainwindow.h"
#include "project/projectmanager.h"
#include "trackview.h"
#include "customtrackview.h"

#include <KStatusBar>


TimelineSearch::TimelineSearch(QObject* parent) :
    QObject(parent)
{
    connect(&m_searchTimer, SIGNAL(timeout()), SLOT(slotEndSearch()));
    m_searchTimer.setSingleShot(true);

    m_findAction = pCore->window()->addAction("project_find", i18n("Find"), this, SLOT(slotInitSearch()), QIcon::fromTheme("edit-find"), Qt::Key_Slash);

    m_findNextAction = pCore->window()->addAction("project_find_next", i18n("Find Next"), this, SLOT(slotFindNext()), QIcon::fromTheme("go-down-search"), Qt::Key_F3);
    m_findNextAction->setEnabled(false);
}

void TimelineSearch::slotInitSearch()
{
    m_findAction->setEnabled(false);
    m_searchTerm.clear();
    pCore->projectManager()->currentTrackView()->projectView()->initSearchStrings();
    pCore->window()->statusBar()->showMessage(i18n("Starting -- find text as you type"));
    m_searchTimer.start(5000);
    qApp->installEventFilter(this);
}

void TimelineSearch::slotEndSearch()
{
    m_searchTerm.clear();
    pCore->window()->statusBar()->showMessage(i18n("Find stopped"), 3000);
    pCore->projectManager()->currentTrackView()->projectView()->clearSearchStrings();
    m_findNextAction->setEnabled(false);
    m_findAction->setEnabled(true);
    qApp->removeEventFilter(this);
}

void TimelineSearch::slotFindNext()
{
    if (pCore->projectManager()->currentTrackView() && pCore->projectManager()->currentTrackView()->projectView()->findNextString(m_searchTerm)) {
        pCore->window()->statusBar()->showMessage(i18n("Found: %1", m_searchTerm));
    } else {
        pCore->window()->statusBar()->showMessage(i18n("Reached end of project"));
    }
    m_searchTimer.start(4000);
}

void TimelineSearch::search()
{
    if (pCore->projectManager()->currentTrackView() && pCore->projectManager()->currentTrackView()->projectView()->findString(m_searchTerm)) {
        m_findNextAction->setEnabled(true);
        pCore->window()->statusBar()->showMessage(i18n("Found: %1", m_searchTerm));
    } else {
        m_findNextAction->setEnabled(false);
        pCore->window()->statusBar()->showMessage(i18n("Not found: %1", m_searchTerm));
    }
}

bool TimelineSearch::eventFilter(QObject *watched, QEvent *event)
{
    // The ShortcutOverride event is emitted for every keyPressEvent, no matter if
    // it is a registered shortcut or not.

    if (watched == pCore->window() && event->type() == QEvent::ShortcutOverride) {

        // Search, or pass event on if no search active or started
        return keyPressEvent(static_cast<QKeyEvent*>(event));

    } else {
        return QObject::eventFilter(watched, event);
    }
}

/**
 * @return true, iff a search operation is in progress or started with the pressed key
 */
bool TimelineSearch::keyPressEvent(QKeyEvent *keyEvent)
{
    if (keyEvent->key() == Qt::Key_Backspace) {
        m_searchTerm = m_searchTerm.left(m_searchTerm.length() - 1);

        if (m_searchTerm.isEmpty()) {
            slotEndSearch();
        } else {
            search();
            m_searchTimer.start(4000);
        }
    } else if (keyEvent->key() == Qt::Key_Escape) {
        slotEndSearch();
    } else if (keyEvent->key() == Qt::Key_Space || !keyEvent->text().trimmed().isEmpty()) {
        m_searchTerm += keyEvent->text();

        search();

        m_searchTimer.start(4000);
    } else {
        return false;
    }

    keyEvent->accept();
    return true;
}
#include "timelinesearch.moc"