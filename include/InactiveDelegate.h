#ifndef INACTIVEDELEGATE_H
#define INACTIVEDELEGATE_H

#include "ActiveDelegate.h"
#include "Constants.h"
#include "GameField.h"

namespace SPR
{
	class InactiveDelegate : public ActiveDelegate
	{
	  public:
		InactiveDelegate(QObject *parent = nullptr);

	  protected:
		virtual void getStyleForField(const GameField &field, QStyleOptionButton &buttonStyle) const;
	};
}	 // namespace SPR
#endif	  // INACTIVEDELEGATE_H
