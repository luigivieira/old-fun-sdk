/*
 * Copyright (C) 2016-2017 Luiz Carlos Vieira (http://www.luiz.vieira.nom.br)
 *
 * This file is part of Fun SDK (FSDK).
 *
 * FSDK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FSDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "videowindow.h"
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QToolBar>

namespace fsdk
{
    /**
     * Main window class.
     */
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        /**
         * Class constructor.
         * @param pParent QWidget with the parent window.
         */
        explicit MainWindow(QWidget *pParent = 0);

	protected:

		/**
		 * Captures the event of window show.
		 * @param pEvent QShowEvent instance with the event data.
		 */
		void showEvent(QShowEvent *pEvent);

		/**
		 * Captures the event of window close.
		 * @param pEvent QCloseEvent instance with the event data.
		 */
		void closeEvent(QCloseEvent *pEvent);

		/**
		 * Sets up the UI elements.
		 */
		void setupUI();

		/**
		 * Refreshes the text in UI elements (so translation changes
		 * can be applied).
		 */
		void refreshUI();
		
	private slots:
	
		/**
		 * Opens a session file selected by the user.
		 */
		void openSession();

		/**
		 * Saves the current session file.
		 */
		void saveSession();

		/**
		 * Saves the current session file with a different file name
		 * selected by the user.
		 */
		void saveSessionAs();

		/**
		 * Quits the application.
		 */
		void quit();

		/**
		 * Shows help for the application.
		 */
		void help();

		/**
		* Shows information about the application.
		*/
		void about();

    private:

		/** Video window that displays the player's face video. */
		VideoWindow *m_pPlayerWindow;

		/** Video window that displays the gameplay video. */
		VideoWindow *m_pGameplayWindow;

		//-------------------------------
		// "File" menu/toolbar
		//-------------------------------

		/** Main menu item called "File". */
		QMenu *m_pFileMenu;

		/** Toolbar for the "File" menu. */
		QToolBar *m_pFileToolbar;

		/** Action called "Open": opens a project file. */
		QAction *m_pOpenAction;

		/** Action called "Save": saves the current project file. */
		QAction *m_pSaveAction;

		/** Action called "Save As": saves the current project file with a different name. */
		QAction *m_pSaveAsAction;

		/** Action called "Exit": terminates the application. */
		QAction *m_pExitAction;

		//-------------------------------
		// "View" menu/toolbar
		//-------------------------------

		/** Main menu item called "View". */
		QMenu *m_pViewMenu;

		/** View menu's submenu called "Windows". */
		QMenu *m_pViewWindowsMenu;

		/** View menu's submenu called "Toolbars". */
		QMenu *m_pViewToolbarsMenu;

		//-------------------------------
		// "Help" menu/toolbar
		//-------------------------------

		/** Main menu item called "Help". */
		QMenu *m_pHelpMenu;

		/** Action called "Help": shows online help. */
		QAction *m_pHelpAction;

		/** Action called "About": shows application information. */
		QAction *m_pAboutAction;

    };
};

#endif // MAINWINDOW_H
