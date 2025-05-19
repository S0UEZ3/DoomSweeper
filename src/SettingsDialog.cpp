#include <include/SettingsDialog.h>

namespace SPR
{

	SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
	{
		// Main layout
		QGridLayout* mainLayout = new QGridLayout(this);

		// Group box
		QGroupBox* groupBox = new QGroupBox(this);
		QFormLayout* formLayout = new QFormLayout(groupBox);

		// Title label
		QLabel* titleLabel = new QLabel(tr("Preferences"), groupBox);
		QFont titleFont;
		titleFont.setPointSize(FONT_SIZE);
		titleLabel->setFont(titleFont);
		formLayout->setWidget(0, QFormLayout::SpanningRole, titleLabel);

		// Width controls
		QLabel* widthLabel = new QLabel(tr("Width"), groupBox);
		m_heightSpinBox = new QSpinBox(groupBox);
		formLayout->setWidget(1, QFormLayout::LabelRole, widthLabel);
		formLayout->setWidget(1, QFormLayout::FieldRole, m_heightSpinBox);

		// Height controls
		QLabel* heightLabel = new QLabel(tr("Height"), groupBox);
		m_widthSpinBox = new QSpinBox(groupBox);
		formLayout->setWidget(2, QFormLayout::LabelRole, heightLabel);
		formLayout->setWidget(2, QFormLayout::FieldRole, m_widthSpinBox);

		// mine controls
		QLabel* mineLabel = new QLabel(tr("mine"), groupBox);
		m_mineSpinBox = new QSpinBox(groupBox);
		formLayout->setWidget(3, QFormLayout::LabelRole, mineLabel);
		formLayout->setWidget(3, QFormLayout::FieldRole, m_mineSpinBox);

		// Button box
		m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, groupBox);
		formLayout->setWidget(4, QFormLayout::SpanningRole, m_buttonBox);

		m_heightSpinBox->setMinimum(SPINBOX_HEIGHT);
		m_widthSpinBox->setMinimum(SPINBOX_WIDTH);

		connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
		connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
		connect(m_heightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateRange()));
		connect(m_widthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateRange()));

		// Add group box to main layout
		mainLayout->addWidget(groupBox, 0, 0);	  // 0 row, 0 column
		setWindowTitle(tr("Preferences"));
		setFixedSize(P_LAYOUT_WIDTH, P_LAYOUT_HEIGHT);
	}

	SettingsDialog::~SettingsDialog() = default;

	bool SettingsDialog::getPreferences(Preferences& prefs, QWidget* parent)
	{
		SettingsDialog dialog(parent);

		dialog.m_heightSpinBox->setValue(prefs.width);
		dialog.m_widthSpinBox->setValue(prefs.height);
		dialog.m_mineSpinBox->setValue(prefs.mine);
		dialog.updateRange();

		if (dialog.exec() == QDialog::Accepted)
		{
			prefs.width = dialog.m_heightSpinBox->value();
			prefs.height = dialog.m_widthSpinBox->value();
			prefs.mine = dialog.m_mineSpinBox->value();
			return true;
		}

		return false;
	}

	void SettingsDialog::updateRange()
	{
		int min = MIN_DENSITY * m_heightSpinBox->value() * m_widthSpinBox->value();
		int max = MAX_DENSITY * m_heightSpinBox->value() * m_widthSpinBox->value();
		m_mineSpinBox->setRange(min, max);
	}

}	 // namespace SPR
