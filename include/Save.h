#ifndef SAVE_H
#define SAVE_H

#include "MineSweeper.h"
#include "Preferences.h"
#include "TopWidget.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <memory>

namespace FileFormat
{
	const QString QUICKSAVE_FILE = "quicksave.ini";
}	 // namespace FileFormat

namespace SPR
{
	class Save : public QObject
	{
		Q_OBJECT

	  public:
		explicit Save(MineSweeper& model, QTimer& timer, Preferences& prefs, QObject* parent = nullptr);

		void setTopWidget(TopWidget* topWidget);

		bool saveGame();
		bool loadGame();

		bool quickSave();
		bool quickLoad();

		static QString quickSavePath();

		int getElapsedTime(const QString& filepath) const;

	  signals:
		void restoreElapsed(int msec);

	  private:
		MineSweeper& _model;
		QTimer& _timer;
		Preferences& _prefs;
		QObject* _parent;
		TopWidget* _topWidget;

		bool serialize(const QString& filepath);
		bool deserialize(const QString& filepath);
	};

}	 // namespace SPR

#endif	  // SAVE_H
