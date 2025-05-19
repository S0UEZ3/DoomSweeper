#include <include/TopWidget.h>

namespace SPR
{

	TopWidget::TopWidget(QWidget* parent) :
		QWidget(parent), m_time(0), m_normalFace(QPixmap(NORMAL_FACE)), m_fearFace(QPixmap(FEAR_FACE)),
		m_happyFace(QPixmap(HAPPY_FACE)), m_sadFace(QPixmap(SAD_FACE))
	{
		// Main layout
		QVBoxLayout* mainLayout = new QVBoxLayout(this);
		mainLayout->setContentsMargins(0, 0, 0, 0);

		// Create frame
		m_frame = new QFrame(this);
		m_frame->setMinimumSize(0, FRAME_HEIGHT);
		m_frame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
		m_frame->setFrameShape(QFrame::StyledPanel);
		m_frame->setFrameShadow(QFrame::Raised);

		// Frame layout
		QHBoxLayout* frameLayout = new QHBoxLayout(m_frame);
		frameLayout->setSizeConstraint(QLayout::SetNoConstraint);

		// Time LCD
		m_lcdTime = new QLCDNumber(m_frame);
		m_lcdTime->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		m_lcdTime->setSegmentStyle(QLCDNumber::Flat);
		m_lcdTime->setFrameShape(QFrame::StyledPanel);
		m_lcdTime->setFrameShadow(QFrame::Sunken);
		m_lcdTime->setMode(QLCDNumber::Dec);

		// First spacer
		m_spacer1 = new QSpacerItem(SPACER_WIDTH, SPACER_HEIGHT, QSizePolicy::Minimum, QSizePolicy::Minimum);

		// Central button
		m_pushButton = new QPushButton(m_frame);
		m_pushButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		m_pushButton->setMinimumSize(BUTTON_SIZE, BUTTON_SIZE);
		m_pushButton->setMaximumSize(BUTTON_SIZE, BUTTON_SIZE);
		m_pushButton->setFlat(false);

		// Second spacer
		m_spacer2 = new QSpacerItem(SPACER_WIDTH, SPACER_HEIGHT, QSizePolicy::Minimum, QSizePolicy::Minimum);

		// mine LCD
		m_lcdMine = new QLCDNumber(m_frame);
		m_lcdMine->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		m_lcdMine->setSegmentStyle(QLCDNumber::Flat);
		m_lcdMine->setFrameShape(QFrame::StyledPanel);
		m_lcdMine->setFrameShadow(QFrame::Sunken);

		// Add widgets to frame layout
		frameLayout->addWidget(m_lcdTime);
		frameLayout->addItem(m_spacer1);
		frameLayout->addWidget(m_pushButton);
		frameLayout->addItem(m_spacer2);
		frameLayout->addWidget(m_lcdMine);

		// Add frame to main layout
		mainLayout->addWidget(m_frame, 0, Qt::AlignVCenter);

		init();
	}

	TopWidget::~TopWidget() = default;

	void TopWidget::incrementTimer()
	{
		m_time++;
		m_lcdTime->display(m_time);
	}

	void TopWidget::resetTimer()
	{
		m_time = 0;
		m_lcdTime->display(m_time);
	}

	void TopWidget::setTimer(int sec)
	{
		m_time = sec;
		m_lcdTime->display(m_time);
	}

	void TopWidget::setMineDisplay(int mineRemained)
	{
		m_lcdMine->display(mineRemained);
	}

	void TopWidget::onPressed()
	{
		m_pushButton->setIcon(m_fearFace);
	}

	void TopWidget::onReleased()
	{
		m_pushButton->setIcon(m_normalFace);
	}

	void TopWidget::onWon()
	{
		m_pushButton->setIcon(m_happyFace);
	}

	void TopWidget::onLost()
	{
		m_pushButton->setIcon(m_sadFace);
	}

	void TopWidget::setDefault()
	{
		m_pushButton->setIcon(m_normalFace);
	}

	void TopWidget::init()
	{
		setDefault();
		m_pushButton->setIconSize(QSize(TOP_ICON_SIZE, TOP_ICON_SIZE));

		connect(m_pushButton, &QPushButton::clicked, this, &TopWidget::buttonClicked);
	}

	int TopWidget::getTime() const
	{
		return m_time;
	}
}	 // namespace SPR
