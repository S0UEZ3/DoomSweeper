#include <include/ActiveDelegate.h>

namespace SPR
{

	ActiveDelegate::ActiveDelegate(QObject *parent) : QAbstractItemDelegate(parent), m_fieldSize(FIELD_SIZE) {}

	void ActiveDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
	{
		GameField field = index.model()->data(index, Qt::UserRole).value< GameField >();
		QStyleOptionButton buttonStyle;
		getGeneralsForField(field, option, buttonStyle);
		getStyleForField(field, buttonStyle);

		if (field.isHighlighted)
		{
			QColor highlightColor = QColor(255, 255, 0, 100);	 // light yellow
			painter->fillRect(option.rect, highlightColor);
		}

		QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonStyle, painter, qobject_cast< QWidget * >(this->parent()));
	}

	QSize ActiveDelegate::sizeHint(const QStyleOptionViewItem & /* option */, const QModelIndex & /* index */) const
	{
		return QSize(m_fieldSize, m_fieldSize);	   // don't need this args here, mentioned just to match method
												   // signature
	}

	void ActiveDelegate::getStyleForField(const GameField &field, QStyleOptionButton &buttonStyle) const
	{
		if (field.isDebug && field.mine)
		{
			buttonStyle.icon = QIcon(QPixmap(CACO_PATH));
		}

		else if (field.discovered && field.neighbours != 0)
		{
			buttonStyle.text = QString::number(field.neighbours);
		}
		else
		{
			if (field.disarmed == 1)
			{
				buttonStyle.icon = QIcon(QPixmap(DISARMED_PATH));
			}
			else if (field.disarmed == 2)
			{
				buttonStyle.icon = QIcon(QPixmap(QUESTION_PATH));
			}
		}
	}

	void ActiveDelegate::getGeneralsForField(const GameField &field, const QStyleOptionViewItem &option, QStyleOptionButton &buttonStyle) const
	{
		buttonStyle.rect = option.rect;
		buttonStyle.iconSize = QSize(ICON_SIZE, ICON_SIZE);
		buttonStyle.state |= QStyle::State_Enabled | option.state;

		if (field.discovered)
		{
			buttonStyle.state |= QStyle::State_Sunken;
		}
		else
		{
			buttonStyle.state |= QStyle::State_Raised;
		}
	}

}	 // namespace SPR
