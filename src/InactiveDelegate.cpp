#include "include/InactiveDelegate.h"

namespace SPR
{

	InactiveDelegate::InactiveDelegate(QObject *parent) : ActiveDelegate(parent) {}

	void InactiveDelegate::getStyleForField(const GameField &field, QStyleOptionButton &buttonStyle) const
	{
		if (field.disarmed > 0 && field.mine == 1)
		{
			buttonStyle.icon = QIcon(QPixmap(DISARMED_PATH));
		}
		else if (field.disarmed > 0 && field.mine == 0)
		{
			buttonStyle.icon = QIcon(QPixmap(DISARMED_RED_PATH));
		}
		else if (field.discovered == 0 && field.mine == 1)
		{
			buttonStyle.icon = QIcon(QPixmap(CACO_PATH));
		}
		else if (field.discovered == 1 && field.mine == 1)
		{
			buttonStyle.icon = QIcon(QPixmap(BLUE_CACO_PATH));
		}
		else if (field.discovered == 1 && field.mine == 0 && field.neighbours != 0)
		{
			buttonStyle.text = QString::number(field.neighbours);
		}
	}
}	 // namespace SPR
