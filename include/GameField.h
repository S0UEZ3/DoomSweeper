#ifndef SFIELD
#define SFIELD

#include <QMetaType>

namespace SPR
{
	struct GameField
	{
		qint8 mine = 0;
		qint8 discovered = 0;
		qint8 disarmed = 0;
		qint8 neighbours = 0;
		bool isDebug = 0;
		bool isHighlighted = false;
	};

}	 // namespace SPR

Q_DECLARE_METATYPE(SPR::GameField);

#endif	  // SFIELD
