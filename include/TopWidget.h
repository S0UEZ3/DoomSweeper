#ifndef TOPWIDGET_H
#define TOPWIDGET_H

#include "Constants.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLCDNumber>
#include <QPixmap>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>

namespace SPR
{

	class TopWidget : public QWidget
	{
		Q_OBJECT

	  public:
		explicit TopWidget(QWidget* parent = nullptr);
		virtual ~TopWidget();

		virtual int getTime() const;

	  signals:
		void buttonClicked();

	  public slots:
		void setMineDisplay(int mineRemained);
		virtual void setTimer(int sec);
		void incrementTimer();
		void resetTimer();
		void onPressed();
		void onReleased();
		void onWon();
		void onLost();
		void setDefault();

	  private:
		void init();

		QFrame* m_frame;
		QLCDNumber* m_lcdTime;
		QLCDNumber* m_lcdMine;
		QPushButton* m_pushButton;
		QSpacerItem* m_spacer1;
		QSpacerItem* m_spacer2;

		int m_time;
		QIcon m_normalFace;
		QIcon m_fearFace;
		QIcon m_happyFace;
		QIcon m_sadFace;
	};

}	 // namespace SPR

#endif	  // TOPWIDGET_H
