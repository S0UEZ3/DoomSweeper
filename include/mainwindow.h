#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Constants.h"
#include "Preferences.h"
#include "Save.h"
#include "SettingsDialog.h"
#include "TableState.h"
#include "TableView.h"
#include "TopWidget.h"

#include <QAction>
#include <QApplication>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QTimer>
#include <QTranslator>

namespace SPR
{

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	  public:
		explicit MainWindow(bool debugMode = false, QWidget* parent = nullptr);
		~MainWindow();

		void closeEvent(QCloseEvent* event);

		void setDebugMode(bool enabled);
		bool isDebugMode() const;

	  public slots:
		void quickSaveGame();
		void saveGameAs();
		void quickLoadGame();
		void loadFrom();
		void newGame();
		void onGameLost();
		void onGameWon();
		void showAboutBox();
		void showPreferences();
		void updateView();

	  private:
		void initTable();
		void initMenubar();
		void initConnections();
		void loadSettings();
		void saveSettings();
		void loadTranslation(const QString& language);
		void changeLanguage(const QString& locale);

		// visuals
		TopWidget* _topWidget;
		TableView* _view;

		// logic
		TableState _model;
		QTimer _timer;
		Preferences _prefs;
		Save _saveSystem;
		bool _debugMode;
	};

}	 // namespace SPR

#endif	  // MAINWINDOW_H
