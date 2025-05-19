#ifndef CONSTANTS
#define CONSTANTS

#include <QString>

namespace SPR
{

	// Global names
	const QString APP = "Doomsweeper";
	const QString ORG = "S0UEEZ3";

	// Icon path
	const QString CACO_PATH = ":/images/small_caco.png";
	const QString BLUE_CACO_PATH = ":/images/small_blue_caco.png";
	const QString DOOM_PATH = ":/images/doomsweeper.png";
	const QString DISARMED_PATH = ":/images/disarmed.png";
	const QString DISARMED_RED_PATH = ":/images/disarmed_red.png";
	const QString QUESTION_PATH = ":/images/question.png";
	const QString SAD_FACE = "://images/sad_32.png";
	const QString NORMAL_FACE = "://images/normal_32.png";
	const QString FEAR_FACE = "://images/fear_32.png";
	const QString HAPPY_FACE = "://images/happy_32.png";

	// Icon size
	const int FIELD_SIZE = 24;
	const int ICON_SIZE = 16;

	// GUI constants
	const int DEFAULT_WINDOW_WIDTH = 276;
	const int DEFAULT_WINDOW_HEIGHT = 327;
	const int MARGIN_SIZE = 11;
	const int DEFAULT_SPACE = 3;
	const int DOUBLE_SPACE = 6;

	// TopWidget constants
	const int MIN_WIDTH = 0;
	const int MIN_HEIGHT = 50;
	const int MAX_WIDTH = 4000;
	const int MAX_HEIGHT = 100;
	const int TOP_ICON_SIZE = 30;
	const int FRAME_HEIGHT = 50;
	const int BUTTON_SIZE = 40;
	const int SPACER_WIDTH = 40;
	const int SPACER_HEIGHT = 20;

	// Preferences layout constants
	const int P_LAYOUT_WIDTH = 248;
	const int P_LAYOUT_HEIGHT = 210;
	const int SPINBOX_WIDTH = 3;
	const int SPINBOX_HEIGHT = 3;
	const int FONT_SIZE = 15;
	const int MIN_HEADER_SIZE = 1;
	const int TWO_PIX_MARGIN = 2;

	// Default game preferences
	const int DEFAULT_HEIGHT = 10;
	const int DEFAULT_WIDTH = 10;
	const int DEFAULT_MINE = 10;

	const int MSG_TIMEOUT = 5000;
	const int ONE_SEC_TICK = 1000;

	const double MIN_DENSITY = 0.05;
	const double MAX_DENSITY = 0.95;

	// Disarming Logic. Field States
	const int PLAYER_NOT_SURE = 2;
	const int FIELD_VISITED = 1;
	const int FIELD_NOT_VISITED = 0;

	// Time
	const int HIGHLIGHT_TIMEOUT = 300;
	const int TWO_SEC_TIMEOUT = 2000;

}	 // namespace SPR

#endif	  // CONSTANTS
