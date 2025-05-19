#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include "ActiveDelegate.h"
#include "InactiveDelegate.h"

#include <QHeaderView>
#include <QMouseEvent>
#include <QTableView>

namespace SPR
{

	class TableView : public QTableView
	{
		Q_OBJECT

	  public:
		TableView(QWidget *parent = nullptr);
		virtual void mouseReleaseEvent(QMouseEvent *event);
		virtual void mousePressEvent(QMouseEvent *event);
		virtual void adjustSizeToContents();

	  private:
		ActiveDelegate m_activeDelegate;
		InactiveDelegate m_inactiveDelegate;

	  public slots:
		void activate();
		void deactivate();

	  signals:
		void rightClicked(const QModelIndex &index);
		void bothClicked(const QModelIndex &index);
		void middleClicked(const QModelIndex &index);
	};

}	 // namespace SPR

#endif	  // TABLEVIEW_H
