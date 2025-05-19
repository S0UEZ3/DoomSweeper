#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "Constants.h"
#include "Preferences.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QSpinBox>

namespace SPR
{

	class SettingsDialog : public QDialog
	{
		Q_OBJECT

	  public:
		~SettingsDialog();
		static bool getPreferences(Preferences& prefs, QWidget* parent = nullptr);

	  public slots:
		void updateRange();

	  private:
		explicit SettingsDialog(QWidget* parent = nullptr);

		// Widgets
		QSpinBox* m_heightSpinBox;
		QSpinBox* m_widthSpinBox;
		QSpinBox* m_mineSpinBox;
		QDialogButtonBox* m_buttonBox;
	};

}	 // namespace SPR

#endif	  // SETTINGSDIALOG_H
