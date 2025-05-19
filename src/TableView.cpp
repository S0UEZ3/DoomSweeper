#include "include/TableView.h"

namespace SPR
{

	TableView::TableView(QWidget *parent) : QTableView(parent), m_activeDelegate(), m_inactiveDelegate()
	{
		setShowGrid(false);
		horizontalHeader()->hide();
		verticalHeader()->hide();
		horizontalHeader()->setMinimumSectionSize(MIN_HEADER_SIZE);
		verticalHeader()->setMinimumSectionSize(MIN_HEADER_SIZE);
		setSelectionMode(QAbstractItemView::NoSelection);
	}

	void TableView::mouseReleaseEvent(QMouseEvent *event)
	{
		QModelIndex index = indexAt(event->pos());

		switch (event->button())
		{
		case Qt::RightButton:
		{
			emit rightClicked(index);
			break;
		}

		case Qt::LeftButton:
		{
			if (event->buttons() & Qt::RightButton)
			{
				emit bothClicked(index);
			}
			else
			{
				QTableView::mouseReleaseEvent(event);
			}
			break;
		}

		default:
		{
			break;
		}
		}
	}

	void TableView::mousePressEvent(QMouseEvent *event)
	{
		switch (event->button())
		{
		case Qt::RightButton:
		{
			break;
		}

		case Qt::LeftButton:
		{
			QTableView::mousePressEvent(event);
			break;
		}

		case Qt::MiddleButton:
		{
			QModelIndex index = indexAt(event->pos());

			if (index.isValid())
			{
				emit middleClicked(index);
			}
			break;
		}

		default:
		{
			break;
		}
		}
	}

	void TableView::adjustSizeToContents()
	{
		resizeColumnsToContents();
		resizeRowsToContents();
		// second row, second column. zero indexation
		int height = rowHeight(1) * model()->rowCount() + TWO_PIX_MARGIN;
		int width = columnWidth(1) * model()->columnCount() + TWO_PIX_MARGIN;
		setFixedSize(width, height);
	}

	void TableView::activate()
	{
		reset();
		setEnabled(true);
		setItemDelegate(&m_activeDelegate);
	}

	void TableView::deactivate()
	{
		setItemDelegate(&m_inactiveDelegate);
		setDisabled(true);
	}

}	 // namespace SPR
