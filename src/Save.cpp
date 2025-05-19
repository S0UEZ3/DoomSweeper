#include "include/Save.h"

namespace SPR
{

	Save::Save(MineSweeper& model, QTimer& timer, Preferences& prefs, QObject* parent) :
		QObject(parent), _model(model), _timer(timer), _prefs(prefs), _parent(parent), _topWidget(nullptr)
	{
	}

	bool Save::saveGame()
	{
		QString filename = QFileDialog::getSaveFileName(
			qobject_cast< QWidget* >(_parent),
			tr("Save Game"),
			QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
			tr("Minesweeper Saves (*.ini)"));

		if (filename.isEmpty())
		{
			return false;
		}

		return serialize(filename);
	}

	bool Save::loadGame()
	{
		QString filename = QFileDialog::getOpenFileName(
			qobject_cast< QWidget* >(_parent),
			tr("Load Game"),
			QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
			tr("Minesweeper Saves (*.ini)"));

		if (filename.isEmpty())
		{
			return false;
		}

		return deserialize(filename);
	}

	bool Save::quickSave()
	{
		return serialize(quickSavePath());
	}

	bool Save::quickLoad()
	{
		return deserialize(quickSavePath());
	}

	QString Save::quickSavePath()
	{
		return FileFormat::QUICKSAVE_FILE;
	}

	bool Save::serialize(const QString& filepath)
	{
		QSettings settings(filepath, QSettings::IniFormat);

		settings.beginGroup("Game");
		settings.setValue("width", _model.width());
		settings.setValue("height", _model.height());
		settings.setValue("mine", _model.totalMineNr());
		settings.endGroup();

		settings.beginGroup("Timer");
		settings.setValue("elapsed", _topWidget->getTime());
		settings.setValue("running", _timer.isActive());
		settings.endGroup();

		settings.beginGroup("Board");
		for (int y = 0; y < _model.height(); ++y)
		{
			for (int x = 0; x < _model.width(); ++x)
			{
				settings.beginGroup(QString("Cell_%1_%2").arg(x).arg(y));
				const GameField& field = _model.field(x, y);
				settings.setValue("mine", field.mine);
				settings.setValue("discovered", field.discovered);
				settings.setValue("disarmed", field.disarmed);
				settings.setValue("neighbours", field.neighbours);
				settings.endGroup();
			}
		}
		settings.endGroup();

		return settings.status() == QSettings::NoError;
	}

	bool Save::deserialize(const QString& filepath)
	{
		QSettings settings(filepath, QSettings::IniFormat);

		if (!settings.contains("Game/width"))
		{
			return false;
		}

		int width = settings.value("Game/width").toInt();
		int height = settings.value("Game/height").toInt();
		int mine = settings.value("Game/mine").toInt();

		_model.reset(width, height, mine);

		settings.beginGroup("Board");
		for (int y = 0; y < height; ++y)
		{
			for (int x = 0; x < width; ++x)
			{
				settings.beginGroup(QString("Cell_%1_%2").arg(x).arg(y));
				GameField& field = _model.field(x, y);
				field.mine = settings.value("mine").toInt();
				field.discovered = settings.value("discovered").toInt();
				field.disarmed = settings.value("disarmed").toInt();
				field.neighbours = settings.value("neighbours").toInt();
				settings.endGroup();
			}
		}
		settings.endGroup();

		int elapsed = settings.value("Timer/elapsed").toInt();

		_topWidget->setTimer(elapsed);

		_prefs.width = width;
		_prefs.height = height;
		_prefs.mine = mine;

		return true;
	}

	void Save::setTopWidget(TopWidget* topWidget)
	{
		_topWidget = topWidget;
	}
}	 // namespace SPR
