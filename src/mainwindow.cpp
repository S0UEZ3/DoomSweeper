#include "include/mainwindow.h"

namespace SPR
{

	MainWindow::MainWindow(bool debugMode, QWidget *parent) :
		QMainWindow(parent), _topWidget(nullptr), _view(nullptr), _model(), _timer(), _prefs(),
		_saveSystem(_model.getMineSweeper(), _timer, _prefs, this), _debugMode(debugMode)
	{
		QSettings settings;
		QString language = settings.value("language", "en_US").toString();

		loadTranslation(language);

		setWindowTitle(APP);
		setWindowIcon(QIcon(QPixmap(DOOM_PATH)));
		setFixedSize(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);	  // window size obviously, width x height

		_model.setDebugMode(debugMode);

		// Central widget and layouts
		QWidget *centralWidget = new QWidget(this);	   //  parent-child memory management system
		QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
		mainLayout->setContentsMargins(MARGIN_SIZE, MARGIN_SIZE, MARGIN_SIZE, MARGIN_SIZE);
		mainLayout->setSpacing(DOUBLE_SPACE);

		// Inner layout
		QVBoxLayout *innerLayout = new QVBoxLayout();
		innerLayout->setSpacing(DEFAULT_SPACE);
		innerLayout->setSizeConstraint(QLayout::SetMinimumSize);

		// Top widget
		_topWidget = new TopWidget(centralWidget);
		_saveSystem.setTopWidget(_topWidget);
		_topWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		_topWidget->setMinimumSize(MIN_WIDTH, MIN_HEIGHT);
		_topWidget->setMaximumSize(MAX_WIDTH, MAX_HEIGHT);

		// Table view
		_view = new TableView(centralWidget);
		_view->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		_view->setStyleSheet("background-color: rgb(222, 222, 222)");

		// Widgets
		innerLayout->addWidget(_topWidget);
		innerLayout->addWidget(_view, 0, Qt::AlignHCenter | Qt::AlignVCenter);
		mainLayout->addLayout(innerLayout);

		setCentralWidget(centralWidget);

		if (QFile::exists(Save::quickSavePath()))
		{
			QMessageBox::StandardButton reply =
				QMessageBox::question(this, tr("Resume Game"), tr("Found auto-saved game. Resume?"), QMessageBox::Yes | QMessageBox::No);

			if (reply == QMessageBox::Yes && _saveSystem.quickLoad())
			{
				_model.resetModel(_prefs.width, _prefs.height, _prefs.mine);
				statusBar()->showMessage(tr("Game resumed from auto-save"), MSG_TIMEOUT);
			}
			else
			{
				QFile::remove(Save::quickSavePath());
				newGame();
			}
		}
		else
		{
			newGame();
		}

		loadSettings();

		initTable();
		initMenubar();
		_topWidget->setTimer(0);
		initConnections();

		setWindowIcon(QIcon(QPixmap(DOOM_PATH)));
		setWindowTitle(APP);
	}

	MainWindow::~MainWindow() = default;

	void MainWindow::loadTranslation(const QString &language)
	{
		QTranslator *translator = new QTranslator(this);

		QString translationPath = ":/translations/" + language + ".qm";

		if (translator->load(translationPath))
		{
			qApp->installTranslator(translator);
		}
		else
		{
			qDebug() << "Translation file for" << language << "not found!";
		}
	}

	void MainWindow::changeLanguage(const QString &locale)
	{
		loadTranslation(locale);
		QSettings settings;
		settings.setValue("language", locale);
		updateView();
	}

	void MainWindow::closeEvent(QCloseEvent *event)
	{
		if (_model.isGameInProgress())
		{
			_saveSystem.quickSave();
		}
		saveSettings();
		event->accept();
	}

	void MainWindow::initTable()
	{
		_view->setModel(&_model);
		_view->activate();
	}

	void MainWindow::initMenubar()
	{
		QMenu *fileMenu = new QMenu(tr("&File"), this);
		menuBar()->addMenu(fileMenu);
		menuBar()->addSeparator();

		QMenu *languageMenu = new QMenu(tr("&Language"), this);
		menuBar()->addMenu(languageMenu);
		QAction *actionEn = new QAction(tr("English"), this);
		QAction *actionRu = new QAction(tr("Русский"), this);
		connect(actionEn, &QAction::triggered, this, [this]() { changeLanguage("en_US"); });
		connect(actionRu, &QAction::triggered, this, [this]() { changeLanguage("ru_RU"); });
		languageMenu->addAction(actionEn);
		languageMenu->addAction(actionRu);

		QAction *quickSaveAction = fileMenu->addAction(tr("Quick Save"));
		quickSaveAction->setShortcut(QKeySequence("Ctrl+S"));
		connect(quickSaveAction, &QAction::triggered, this, &MainWindow::quickSaveGame);

		QAction *quickLoadAction = fileMenu->addAction(tr("Quick Load"));
		quickLoadAction->setShortcut(QKeySequence("Ctrl+L"));
		connect(quickLoadAction, &QAction::triggered, this, &MainWindow::quickLoadGame);

		QAction *saveAsAction = fileMenu->addAction(tr("Save As"));
		saveAsAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
		connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveGameAs);

		QAction *loadFrom = fileMenu->addAction(tr("Load From"));
		saveAsAction->setShortcut(QKeySequence("Ctrl+Shift+L"));
		connect(loadFrom, &QAction::triggered, this, &MainWindow::loadFrom);

		QAction *newGameAction = fileMenu->addAction(tr("&New game"));
		newGameAction->setShortcut(QKeySequence::New);
		connect(newGameAction, &QAction::triggered, this, &MainWindow::newGame);

		QAction *preferencesAction = fileMenu->addAction(tr("Preferences"));
		preferencesAction->setShortcut(QKeySequence("Ctrl+P"));
		connect(preferencesAction, &QAction::triggered, this, &MainWindow::showPreferences);

		QAction *quitAction = fileMenu->addAction(tr("Exit"));
		quitAction->setShortcut(QKeySequence("Esc"));
		connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

		QMenu *helpMenu = new QMenu(tr("&Help"), this);
		menuBar()->addMenu(helpMenu);

		QAction *aboutAction = helpMenu->addAction(tr("&About"));
		connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutBox);

		// Debug stuff
		QMenu *debugMenu = new QMenu(tr("&Debug"), this);
		menuBar()->addMenu(debugMenu);
		QAction *debugAction = debugMenu->addAction(tr("Debug Mode"));
		debugAction->setCheckable(true);
		debugAction->setChecked(_debugMode);
		debugAction->setShortcut(QKeySequence("Ctrl+D"));
		connect(debugAction, &QAction::triggered, this, &MainWindow::setDebugMode);
	}

	void MainWindow::initConnections()
	{
		// TopWidget
		connect(_view, &TableView::pressed, _topWidget, &TopWidget::onPressed);
		connect(_view, &TableView::clicked, _topWidget, &TopWidget::onReleased);
		connect(_view, &TableView::bothClicked, _topWidget, &TopWidget::onReleased);
		connect(&_timer, &QTimer::timeout, _topWidget, &TopWidget::incrementTimer);
		connect(&_model, &TableState::mineDisplay, _topWidget, &TopWidget::setMineDisplay);
		connect(&_model, &TableState::gameLost, _topWidget, &TopWidget::onLost);
		connect(&_model, &TableState::gameWon, _topWidget, &TopWidget::onWon);
		connect(&_model, &TableState::gameStarted, &_timer, [this]() { _timer.start(ONE_SEC_TICK); });

		// TableModel
		connect(_view, &TableView::clicked, &_model, &TableState::onTableClicked);
		connect(_view, &TableView::rightClicked, &_model, &TableState::onRightClicked);
		connect(_view, &TableView::bothClicked, &_model, &TableState::onBothClicked);
		connect(_view, &TableView::middleClicked, &_model, &TableState::onMiddleClicked);

		// MainWindow
		connect(&_model, &TableState::gameLost, this, &MainWindow::onGameLost);
		connect(&_model, &TableState::gameWon, this, &MainWindow::onGameWon);
		connect(_topWidget, &TopWidget::buttonClicked, this, &MainWindow::newGame);
	}

	void MainWindow::newGame()
	{
		_topWidget->resetTimer();
		_model.resetModel(_prefs.height, _prefs.width, _prefs.mine);
		_view->setModel(&_model);
		_view->activate();
		_topWidget->setDefault();
		statusBar()->showMessage(tr("Good luck!"), MSG_TIMEOUT);
		updateView();
	}

	void MainWindow::quickSaveGame()
	{
		if (_saveSystem.quickSave())
		{
			statusBar()->showMessage(_saveSystem.quickSavePath().toStdString().c_str(), MSG_TIMEOUT);
		}
	}

	void MainWindow::saveGameAs()
	{
		if (_saveSystem.saveGame())
		{
			statusBar()->showMessage(tr("Game saved"), MSG_TIMEOUT);
		}
	}

	void MainWindow::quickLoadGame()
	{
		if (_saveSystem.quickLoad())
		{
			statusBar()->showMessage(tr("Game loaded"), MSG_TIMEOUT);
		}
	}

	void MainWindow::loadFrom()
	{
		if (_saveSystem.loadGame())
		{
			statusBar()->showMessage(tr("Game loaded"), MSG_TIMEOUT);
		}
	}

	void MainWindow::onGameLost()
	{	 // TODO: save stats
		_timer.stop();
		_view->deactivate();
		statusBar()->showMessage(tr("Unfortunately, you died."), MSG_TIMEOUT);
	}

	void MainWindow::onGameWon()
	{	 // TODO: save stats
		_timer.stop();
		_view->deactivate();
		statusBar()->showMessage(tr("You won!"), MSG_TIMEOUT);
	}

	void MainWindow::showPreferences()
	{
		bool accepted = SettingsDialog::getPreferences(_prefs, this);
		if (accepted)
		{
			newGame();
		}
	}

	void MainWindow::showAboutBox()
	{
		QMessageBox::about(this, APP, tr("Numbers is how many mines are around"));	  // TODO: json with rules
	}

	void MainWindow::updateView()
	{
		_view->adjustSizeToContents();
		layout()->setSizeConstraint(QLayout::SetFixedSize);
	}

	void MainWindow::loadSettings()
	{
		QSettings settings;
		_prefs.width = settings.value("width", int(DEFAULT_WIDTH)).toInt();
		_prefs.height = settings.value("height", int(DEFAULT_HEIGHT)).toInt();
		_prefs.mine = settings.value("mine", int(DEFAULT_MINE)).toInt();
		_debugMode = settings.value("debugMode", false).toBool();
		setDebugMode(_debugMode);
	}

	void MainWindow::saveSettings()
	{
		QSettings settings;
		settings.setValue("width", _prefs.width);
		settings.setValue("height", _prefs.height);
		settings.setValue("mine", _prefs.mine);
		settings.setValue("debugMode", _debugMode);
	}

	void MainWindow::setDebugMode(bool enabled)
	{
		_debugMode = enabled;
		_model.setDebugMode(enabled);
		statusBar()->showMessage(enabled ? "Debug mode ON" : "Debug mode OFF", TWO_SEC_TIMEOUT);
	}

	bool MainWindow::isDebugMode() const
	{
		return _debugMode;
	}
}	 // namespace SPR
