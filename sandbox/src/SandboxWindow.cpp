#include "SandboxWindow.hpp"

#include <oclero/qlementine/style/QlementineStyle.hpp>
#include <oclero/qlementine/utils/StateUtils.hpp>
#include <oclero/qlementine/utils/PrimitiveUtils.hpp>
#include <oclero/qlementine/utils/ImageUtils.hpp>
#include <oclero/qlementine/widgets/CommandLinkButton.hpp>
#include <oclero/qlementine/widgets/SegmentedControl.hpp>
#include <oclero/qlementine/widgets/IconWidget.hpp>
#include <oclero/qlementine/widgets/Expander.hpp>
#include <oclero/qlementine/widgets/Popover.hpp>
#include <oclero/qlementine/widgets/NavigationBar.hpp>
#include <oclero/qlementine/widgets/Switch.hpp>
#include <oclero/qlementine/widgets/PopoverButton.hpp>
#include <oclero/qlementine/widgets/StatusBadgeWidget.hpp>
#include <oclero/qlementine/widgets/LineEdit.hpp>

#include <QFileSystemWatcher>
#include <QContextMenuEvent>
#include <QShortcut>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QRadioButton>
#include <QToolBar>
#include <QTreeWidget>
#include <QMenuBar>
#include <QToolButton>
#include <QListWidget>
#include <QTableWidget>
#include <QGroupBox>
#include <QScrollArea>
#include <QCheckBox>
#include <QButtonGroup>
#include <QStandardItemModel>
#include <QProgressBar>
#include <QHeaderView>
#include <QApplication>

namespace oclero::qlementine::sandbox {
class ContextMenuEventFilter : public QObject {
private:
  std::function<bool(QContextMenuEvent* evt)> _cb;

public:
  ContextMenuEventFilter(QObject* parent, const std::function<bool(QContextMenuEvent*)>&& cb)
    : QObject(parent)
    , _cb(std::move(cb)) {
    parent->installEventFilter(this);
  }

protected:
  virtual bool eventFilter(QObject* watched, QEvent* evt) override {
    if (evt->type() == QEvent::ContextMenu && _cb) {
      auto* const derivedEvent = static_cast<QContextMenuEvent*>(evt);
      return _cb(derivedEvent);
    }

    return QObject::eventFilter(watched, evt);
  }
};

class CustomPaintWidget : public QWidget {
  using QWidget::QWidget;

  void paintEvent(QPaintEvent*) override {
    const auto text = QString("A very long text than can be elided because it is too long.");
    const auto fm = this->fontMetrics();
    const auto totalW = this->width();
    const auto flags = Qt::AlignCenter;
    auto y = 0;

    const auto rect1 = fm.boundingRect(text);
    const auto rect2 = fm.boundingRect(QRect(), Qt::AlignCenter, text, 0, nullptr);
    const auto rect3 = fm.tightBoundingRect(text);
    const auto hAdvance = fm.horizontalAdvance(text);

    const auto textW1 = rect1.width();
    const auto textW2 = rect2.width();
    const auto textW3 = rect3.width();
    const auto textW4 = hAdvance;

    const auto availableW1 = std::min(textW1, totalW);
    const auto availableW2 = std::min(textW2, totalW);
    const auto availableW3 = std::min(textW3, totalW);
    const auto availableW4 = std::min(textW4, totalW);

    const auto textH1 = rect1.height();
    const auto textH2 = rect2.height();
    const auto textH3 = rect3.height();
    const auto textH4 = fm.height();

    QPainter p(this);
    p.fillRect(this->rect(), Qt::red);

    p.setBrush(Qt::NoBrush);

    const auto text1 = fm.elidedText(text, Qt::ElideRight, availableW1, Qt::TextSingleLine);
    p.setPen(Qt::black);
    p.drawText((totalW - availableW1) / 2, y, availableW1, textH1, flags, text1);
    p.setPen(Qt::white);
    p.drawText(0, y, totalW, textH1, 0, QString("boundingRect %1x%2").arg(textW1).arg(textH1));
    y += textH1;

    const auto text2 = fm.elidedText(text, Qt::ElideRight, availableW2, Qt::TextSingleLine);
    p.setPen(Qt::black);
    p.drawText((totalW - availableW1) / 2, y, availableW2, textH2, flags, text2);
    p.setPen(Qt::white);
    p.drawText(0, y, totalW, textH2, 0, QString("boundingRect2 %1x%2").arg(textW2).arg(textH2));
    y += textH2;

    const auto text3 = fm.elidedText(text, Qt::ElideRight, availableW3, Qt::TextSingleLine);
    p.setPen(Qt::black);
    p.drawText((totalW - availableW3) / 2, y, availableW3, textH3, flags, text3);
    p.setPen(Qt::white);
    p.drawText(0, y, totalW, textH3, 0, QString("tightBoundingRect %1x%2").arg(textW3).arg(textH3));
    y += textH3;

    const auto text4 = fm.elidedText(text, Qt::ElideRight, availableW4, Qt::TextSingleLine);
    p.setPen(Qt::black);
    p.drawText((totalW - availableW4) / 2, y, availableW4, textH4, flags, text4);
    p.setPen(Qt::white);
    p.drawText(0, y, totalW, textH4, 0, QString("horizontalAdvance %1x%2").arg(textW4).arg(textH4));
  }
};

class RoundedTriangleWidget : public QWidget {
  using QWidget::QWidget;

public:
  double radius() const {
    return _r;
  }

  void setRadius(double r) {
    _r = r;
    update();
  }

  QSize sizeHint() const override {
    return QSize(100, 100);
  }

  void paintEvent(QPaintEvent*) override {
    const auto rect = this->rect();
    QPainter p(this);
    p.fillRect(rect, Qt::red);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::yellow);
    drawRoundedTriangle(&p, rect, _r);

    p.setBrush(Qt::NoBrush);
    p.setPen(Qt::black);
    p.drawText(rect, Qt::AlignCenter, QString::number(_r));
  }

private:
  double _r{ 4. };
};

class CustomBgWidget : public QWidget {
public:
  using QWidget::QWidget;

  QColor bgColor{ Qt::red };
  QSize customSizeHint{ -1, -1 };
  bool showBounds{ true };

  QSize sizeHint() const override {
    if (customSizeHint.isValid())
      return customSizeHint;
    else
      return QWidget::sizeHint();
  }

  QSize minimumSizeHint() const override {
    return QSize(0, 0);
  }

protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.fillRect(rect(), bgColor);

    if (showBounds) {
      qlementine::drawRectBorder(&p, rect(), Qt::black, 1.0);
    }
  }
};

struct SandboxWindow::Impl {
  Impl(SandboxWindow& o)
    : owner(o) {
#if 0
    {
      QSettings qSettings;
      lastJsonThemePath = qSettings.value(QStringLiteral("lastJsonThemePath"), QString{}).toString();
      if (lastJsonThemePath.isEmpty()) {
        showFileDialog();
      }

      QObject::connect(&fileWatcher, &QFileSystemWatcher::fileChanged, &owner, [this](const QString& path) {
        if (qlementineStyle && path == lastJsonThemePath) {
          qlementineStyle->setThemeJsonPath(lastJsonThemePath);
        }
      });
      if (!lastJsonThemePath.isEmpty()) {
        fileWatcher.addPath(lastJsonThemePath);
      }
    }
#endif
  }

#if 0
  void showFileDialog() {
    auto const& dir = lastJsonThemePath.isEmpty()
                        ? QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DesktopLocation)
                        : lastJsonThemePath;
    auto const filename = QFileDialog::getOpenFileName(nullptr, tr("Open"), dir, QStringLiteral("*.json"));
    if (!filename.isEmpty()) {
      fileWatcher.removePath(lastJsonThemePath);
      lastJsonThemePath = filename;
      // Save path to settings.
      {
        QSettings qSettings;
        qSettings.setValue(QStringLiteral("lastJsonThemePath"), filename);
      }
      // Watch for changes.
      fileWatcher.addPath(lastJsonThemePath);

      if (reloadJsonAction) {
        reloadJsonAction->setEnabled(!lastJsonThemePath.isEmpty());
        reloadJsonAction->setToolTip(lastJsonThemePath);
      }
#endif

  void setupMenuBar() {
    //auto* menuBar = owner.menuBar();

    //// Load JSON file.
    //auto* const loadJsonAction = new QAction("Load &JSON theme", menuBar);
    //loadJsonAction->setShortcut(QKeySequence::Open);
    //QObject::connect(loadJsonAction, &QAction::triggered, &owner, [this]() { showFileDialog(); });

    //// Reload JSON file.
    //reloadJsonAction = new QAction("&Reload last JSON theme", menuBar);
    //reloadJsonAction->setShortcut(QKeySequence::Refresh);
    //QObject::connect(reloadJsonAction, &QAction::triggered, &owner, [this]() {
    //  if (qlementineStyle) {
    //    qlementineStyle->setThemeJsonPath(lastJsonThemePath);
    //  }
    //});
    //reloadJsonAction->setEnabled(!lastJsonThemePath.isEmpty());
    //reloadJsonAction->setToolTip(lastJsonThemePath);

    //// Quit.
    //auto* const closeAction = new QAction("&Quit", menuBar);
    //closeAction->setShortcuts({ QKeySequence::Close, QKeySequence{ Qt::Key_Escape } });
    //closeAction->setMenuRole(QAction::QuitRole);
    //QObject::connect(closeAction, &QAction::triggered, &owner, []() { QApplication::quit(); });

    //// File menu
    //auto* const fileMenu = menuBar->addMenu("&File");
    //fileMenu->addAction(loadJsonAction);
    //fileMenu->addAction(reloadJsonAction);
    //fileMenu->addAction(closeAction);
  }

  void setupShortcuts() {
    auto* enableShortcut = new QShortcut(Qt::CTRL + Qt::Key_E, &owner);
    enableShortcut->setAutoRepeat(false);
    enableShortcut->setContext(Qt::ShortcutContext::ApplicationShortcut);
    QObject::connect(enableShortcut, &QShortcut::activated, enableShortcut, [this]() {
      // Disable al widgets.
      if (windowContent) {
        windowContent->setEnabled(!windowContent->isEnabled());
      }
      if (toolbar) {
        toolbar->setEnabled(!toolbar->isEnabled());
      }
    });

    auto* themeShortcut = new QShortcut(Qt::CTRL + Qt::Key_T, &owner);
    themeShortcut->setAutoRepeat(false);
    themeShortcut->setContext(Qt::ShortcutContext::ApplicationShortcut);
    QObject::connect(themeShortcut, &QShortcut::activated, themeShortcut, [this]() {
      const auto light = QStringLiteral(":/light.json");
      const auto dark = QStringLiteral(":/dark.json");
      if (lastJsonThemePath == dark) {
        lastJsonThemePath = light;
        qlementineStyle->setThemeJsonPath(light);
      } else {
        lastJsonThemePath = dark;
        qlementineStyle->setThemeJsonPath(dark);
      }
    });

    auto* focusShortcut = new QShortcut(Qt::CTRL + Qt::Key_F, &owner);
    focusShortcut->setAutoRepeat(false);
    focusShortcut->setContext(Qt::ShortcutContext::ApplicationShortcut);
    QObject::connect(focusShortcut, &QShortcut::activated, focusShortcut, [this]() {
      if (auto* w = qApp->focusWidget()) {
        w->clearFocus();
      } else {
        if (const auto* activeWidget = qApp->activeWindow()) {
          const auto widgets = activeWidget->findChildren<QWidget*>();
          for (auto* w : widgets) {
            if (w->isEnabled() && w->focusPolicy() != Qt::NoFocus) {
              w->setFocus();
              break;
            }
          }
        }
      }
    });

    /*auto* quitShortcut = new QShortcut(Qt::Key_Escape, &owner);
    quitShortcut->setAutoRepeat(false);
    quitShortcut->setContext(Qt::ShortcutContext::ApplicationShortcut);
    QObject::connect(quitShortcut, &QShortcut::activated, quitShortcut, [this]() { QApplication::quit(); });*/
  }

  void setupUi_basicWidgets() {
    auto* scrollArea = new QScrollArea(&owner);
    auto* scrollAreaContent = new QWidget(scrollArea);
    scrollAreaContent->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    windowContentLayout = new QVBoxLayout(scrollAreaContent);
    windowContent = scrollAreaContent;

#if 0
    {
      auto* label = new QLabel(windowContent);
      label->setWordWrap(true);
      label->setText("Press CTRL+E to enable/disable widgets, and CTRL+T to change theme.");
      windowContentLayout->addWidget(label);
    }
#endif
#if 0
    {
      auto* button = new QPushButton(windowContent);
      button->setText("Button with a very long text that can be elided");
      button->setIcon(QIcon(":/refresh.svg"));
      button->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      windowContentLayout->addWidget(button);
    }
#endif
#if 0
    {
      // Text, fixed size
      auto* button = new QPushButton(windowContent);
      button->setText("Button");
      button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      windowContentLayout->addWidget(button);
    }
    {
      // Icon, fixed size
      auto* button = new QPushButton(windowContent);
      button->setIcon(QIcon(":/refresh.svg"));
      button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      windowContentLayout->addWidget(button);
    }
    {
      // Text+Icon, fixed size
      auto* button = new QPushButton(windowContent);
      button->setText("Button");
      button->setIcon(QIcon(":/refresh.svg"));
      button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      windowContentLayout->addWidget(button);
    }
    {
      // Text+Icon+Menu, fixed size
      auto* button = new QPushButton(windowContent);
      button->setText("Button");
      button->setIcon(QIcon(":/refresh.svg"));
      button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

      auto* menu = new QMenu(button);
      for (auto i = 0; i < 3; ++i) {
        menu->addAction(new QAction(QString("Action %1").arg(i), menu));
      }
      button->setMenu(menu);
      windowContentLayout->addWidget(button);
    }
    // ------
    {
      // Text, expanding size.
      auto* button = new QPushButton(windowContent);
      button->setText("Button");
      windowContentLayout->addWidget(button);
    }
    {
      // Icon, expanding size.
      auto* button = new QPushButton(windowContent);
      button->setIcon(QIcon(":/refresh.svg"));
      windowContentLayout->addWidget(button);
    }
    {
      // Text+Icon, expanding size.
      auto* button = new QPushButton(windowContent);
      button->setText("Button");
      button->setIcon(QIcon(":/refresh.svg"));
      windowContentLayout->addWidget(button);
    }
    {
      // Text+Icon+Menu, expanding size
      auto* button = new QPushButton(windowContent);
      button->setText("Button");
      button->setIcon(QIcon(":/refresh.svg"));

      auto* menu = new QMenu("ButtonMenu");
      for (auto i = 0; i < 3; ++i) {
        menu->addAction(new QAction(QString("Action %1").arg(i), menu));
      }
      button->setMenu(menu);
      windowContentLayout->addWidget(button);
    }
#endif
#if 1
    {
      for (auto i = 0; i < 2; ++i) {
        auto* checkbox = new QCheckBox(windowContent);
        checkbox->setChecked(true);
        checkbox->setIcon(QIcon(":/refresh.svg"));
        checkbox->setText(QString("Checkbox %1 with a very long text").arg(i));
        checkbox->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        windowContentLayout->addWidget(checkbox);
      }
    }
#endif
#if 1
    {
      auto* radioGroup = new QButtonGroup(windowContent);

      for (auto i = 0; i < 2; ++i) {
        auto* radiobutton = new QRadioButton(windowContent);
        radiobutton->setChecked(true);
        radiobutton->setIcon(QIcon(":/refresh.svg"));
        radiobutton->setText(QString("RadioButton %1 with a very long text").arg(i));
        radiobutton->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
        radioGroup->addButton(radiobutton);
        windowContentLayout->addWidget(radiobutton);
      }
    }
#endif
#if 1
    {
      const QIcon icon(":/plus_24.svg");
      auto* button = new CommandLinkButton(windowContent);
      button->setText("First Line with a very long text that should be cropped");
      button->setDescription("Second Line that could be very long and should be cropped");
      button->setIcon(icon);
      button->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      windowContentLayout->addWidget(button);
    }
#endif
#if 1
    {
      constexpr auto min = 0;
      constexpr auto max = 100;
      constexpr auto val = 5;
      constexpr auto singleStep = (max - min) / max;
      constexpr auto pageStep = (max - min) / 10;

      auto* progressBar = new QProgressBar(windowContent);
      progressBar->setMaximum(max);
      progressBar->setMinimum(min);
      progressBar->setValue(val);
      progressBar->setTextVisible(true);
      progressBar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      windowContentLayout->addWidget(progressBar);

      auto* slider = new QSlider(windowContent);
      slider->setOrientation(Qt::Orientation::Horizontal);
      slider->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      slider->setMinimum(min);
      slider->setMaximum(max);
      slider->setPageStep(pageStep);
      slider->setSingleStep(singleStep);
      slider->setValue(val);
      QObject::connect(slider, &QSlider::valueChanged, progressBar, &QProgressBar::setValue);
      windowContentLayout->addWidget(slider);
    }
#endif
#if 0
    {
      auto* slider = new QSlider(windowContent);
      slider->setOrientation(Qt::Orientation::Horizontal);
      slider->setMinimum(0);
      slider->setMaximum(10);
      slider->setPageStep(1);
      slider->setSingleStep(1);
      slider->setValue(5);
      //slider->setEnabled(false);
      slider->setTickPosition(QSlider::TickPosition::TicksAbove);
      slider->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      windowContentLayout->addWidget(slider);
    }
#endif
#if 1
    {
      auto* lineEdit = new QLineEdit(windowContent);
      lineEdit->setText("Text");
      lineEdit->setPlaceholderText("Placeholder");
      lineEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      windowContentLayout->addWidget(lineEdit);
      lineEdit->setClearButtonEnabled(true);
    }
#endif
#if 0
    {
      auto* dial = new QDial(windowContent);
      dial->setOrientation(Qt::Orientation::Horizontal);
      dial->setMinimum(0);
      dial->setMaximum(100);
      dial->setPageStep(10);
      dial->setSingleStep(1);
      dial->setValue(5);
      dial->setNotchesVisible(true);
      dial->setFixedSize(48, 48);
      windowContentLayout->addWidget(dial);
    }
#endif
#if 1
    {
      auto* spinbox = new QSpinBox(windowContent);
      spinbox->setMinimum(0);
      spinbox->setMaximum(100);
      spinbox->setValue(50);
      spinbox->setSingleStep(1);
      spinbox->setSuffix("km/h");
      spinbox->setPrefix("$");
      spinbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      windowContentLayout->addWidget(spinbox);
    }
#endif
#if 1
    {
      auto* combobox = new QComboBox(windowContent);
      combobox->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      combobox->setFocusPolicy(Qt::NoFocus);
      for (auto i = 0; i < 4; ++i) {
        combobox->addItem(QIcon(":/refresh.svg"), QString("ComboBox item %1").arg(i));
      }
      auto* model = qobject_cast<QStandardItemModel*>(combobox->model());
      auto* item = model->item(2);
      item->setEnabled(false);

      windowContentLayout->addWidget(combobox);
    }
#endif
#if 0
    {
      auto* listView = new QListWidget(windowContent);
      listView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
      //listView->setAlternatingRowColors(true);

      for (auto i = 0; i < 6; ++i) {
        auto* item = new QListWidgetItem(QIcon(":/refresh.svg"), QString("Item #%1 with very long text that can be elided").arg(i), listView);
        item->setFlags(item->flags() | Qt::ItemFlag::ItemIsUserCheckable);
        item->setCheckState(i % 2 ? Qt ::CheckState::Checked : Qt::CheckState::Unchecked);

        listView->addItem(item);
      }
      listView->item(0)->setSelected(true);
      windowContentLayout->addWidget(listView);
    }
#endif
#if 1
    {
      auto* treeWidget = new QTreeWidget(windowContent);
      treeWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
      treeWidget->setAlternatingRowColors(false);
      treeWidget->setColumnCount(1);
      treeWidget->setHeaderHidden(true);
      treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
      QlementineStyle::setAutoIconColorEnabled(treeWidget, false);

      for (auto i = 0; i < 3; ++i) {
        auto* root = new QTreeWidgetItem(treeWidget);
        root->setText(0, QString("Root %1").arg(i + 1));
        root->setIcon(0, QIcon(":/scene_object.svg"));
        root->setText(1, QString("Column 2 of Root %1").arg(i + 1));

        for (auto j = 0; j < 3; ++j) {
          auto* child = new QTreeWidgetItem(root);
          child->setText(0, QString("Child %1 of Root %2").arg(j).arg(i));
          child->setIcon(0, j == 2 ? QIcon(":/scene_light.svg") : QIcon(":/scene_object.svg"));
          child->setText(1, QString("Column 2 of Child %1 of Root %2").arg(j).arg(i));

          for (auto k = 0; k < 3; ++k) {
            auto* subChild = new QTreeWidgetItem(child);
            subChild->setText(0, QString("Child %1 of Child %2 of Root %3").arg(k).arg(j).arg(i));
            subChild->setIcon(0, QIcon(":/scene_material.svg"));
            subChild->setText(1, QString("Column 2 of Child %1 of Child %2 of Root %3").arg(k).arg(j).arg(i));
          }
        }
      }

      treeWidget->topLevelItem(0)->setSelected(true), windowContentLayout->addWidget(treeWidget);
    }
#endif
#if 1
    {
      auto* menuBar = owner.menuBar();
      // NB: it looks like MacOS' native menu bar has an issue with QIcon, so we have to force
      // it to generate icons for High-DPI screens.
      const auto icon = makeIconFromSvg(":/refresh.svg", owner.iconSize());

      for (auto i = 0; i < 5; ++i) {
        auto* menu = menuBar->addMenu(QString("Menu &%1").arg(i));

        for (auto j = 0; j < 10; ++j) {
          auto* action = menu->addAction(icon, QString("Menu %1 - Action &%2").arg(i).arg(j));

          if (j == 0) {
            auto* subMenu = new QMenu(menuBar);

            auto* subActionGroup = new QActionGroup(subMenu);
            for (auto k = 0; k < 6; ++k) {
              auto* subAction = subMenu->addAction(icon, QString("SubMenu %1 - Action &%2").arg(j).arg(k));

              if (k % 2 == 0) {
                subAction->setEnabled(false);
              }
              subActionGroup->addAction(subAction);
              subAction->setCheckable(true);
            }

            action->setMenu(subMenu);
          } else if (j == 1) {
            action->setCheckable(true);
            action->setChecked(true);
          } else if (j % 2 == 0) {
            const auto key_number = (Qt::Key)(Qt::Key_0 + j);
            action->setShortcut(QKeySequence(Qt::CTRL + (Qt::Key_0 + key_number)));
          } else if (j % 3 == 0) {
            const auto key_number = (Qt::Key)(Qt::Key_0 + j);
            action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::ALT + (Qt::Key_0 + key_number)));
          } else if (j % 5 == 0) {
            action->setEnabled(false);
          }
        }
      }
    }
#endif
#if 0
    {
      auto* toolButton = new QToolButton(toolbar);
      toolButton->setIcon(QIcon(":/refresh.svg"));
      toolButton->setText(QString("Button with a very long text that can be elided"));
      toolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
      toolButton->setCheckable(true);
      toolButton->setChecked(true);
      toolButton->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      windowContentLayout->addWidget(toolButton);
    }
#endif
#if 1
    {
      const auto icon = QIcon(":/refresh.svg");

      toolbar = owner.addToolBar("ToolBar name");
      //toolbar->set
      toolbar->setAllowedAreas(Qt::ToolBarArea::TopToolBarArea);
      toolbar->setMovable(false);
      toolbar->setFloatable(false);
      toolbar->setIconSize(QSize(16, 16));
      toolbar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonFollowStyle);

      auto count = 0;

      // Button 1: Icon only
      {
        auto* toolButton = new QToolButton(toolbar);
        toolButton->setIcon(icon);
        toolButton->setText(QString("Button"));
        toolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        count++;
        toolbar->addWidget(toolButton);
      }
      // Button 2: Text only
      {
        auto* toolButton = new QToolButton(toolbar);
        toolButton->setIcon(icon);
        toolButton->setText(QString("Button"));
        toolButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        count++;
        toolbar->addWidget(toolButton);
      }
      // Button 3: Icon and Text.
      {
        auto* toolButton = new QToolButton(toolbar);
        toolButton->setIcon(icon);
        toolButton->setText(QString("Button"));
        toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        count++;
        toolbar->addWidget(toolButton);
      }
      // Button 4: Icon and Text, checkable.
      {
        auto* toolButton = new QToolButton(toolbar);
        toolButton->setIcon(icon);
        toolButton->setText(QString("Button"));
        toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toolButton->setCheckable(true);
        toolButton->setChecked(true);
        count++;
        toolbar->addWidget(toolButton);
      }
      // Button 5: Icon only + menu
      {
        auto* toolButton = new QToolButton(toolbar);
        toolButton->setIcon(icon);
        toolButton->setText(QString("Button"));
        toolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
        auto* subMenu = new QMenu("Menu title", toolButton);
        toolButton->setMenu(subMenu);
        subMenu->addAction(icon, "Sub Action 1");
        subMenu->addAction(icon, "Sub Action 2");
        count++;
        toolbar->addWidget(toolButton);
      }

      // Button 6: Text only + menu
      {
        auto* toolButton = new QToolButton(toolbar);
        toolButton->setIcon(icon);
        toolButton->setText(QString("Button"));
        toolButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
        auto* subMenu = new QMenu("Menu title", toolButton);
        toolButton->setMenu(subMenu);
        subMenu->addAction(icon, "Sub Action 1");
        subMenu->addAction(icon, "Sub Action 2");
        count++;
        toolbar->addWidget(toolButton);
      }

      // Button 6: Icon and Text + menu
      {
        auto* toolButton = new QToolButton(toolbar);
        toolButton->setIcon(icon);
        toolButton->setText(QString("Button"));
        toolButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        auto* subMenu = new QMenu("Menu title", toolButton);
        toolButton->setMenu(subMenu);
        subMenu->addAction(icon, "Sub Action 1");
        subMenu->addAction(icon, "Sub Action 2");
        count++;
        toolbar->addWidget(toolButton);
      }
    }
#endif
#if 1
    {
      const QIcon icon(":/scene_object.svg");
      const QIcon icon2(":/scene_light.svg");
      auto* tabBar = new QTabBar(windowContent);
      tabBar->setFocusPolicy(Qt::NoFocus);
      tabBar->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      tabBar->setTabsClosable(true);
      QlementineStyle::setAutoIconColorEnabled(tabBar, false);
      tabBar->setMovable(true);
      tabBar->setExpanding(/*true*/ false);
      tabBar->setChangeCurrentOnDrag(true);
      //tabBar->setUsesScrollButtons(false);
      windowContentLayout->addWidget(tabBar);
      //windowContentLayout->setAlignment(tabBar, Qt::AlignLeft);

      for (auto i = 0; i < 10; ++i) {
        auto tabText = QString(i % 2 ? "Tab with a very long text %1" : "Tab short text %1").arg(i);
        if (i % 2 == 0) {
          tabBar->addTab(icon, tabText);
        } else {
          tabBar->addTab(icon2, tabText);
        }
        tabBar->setTabToolTip(i, tabText);
      }

      tabBar->setCurrentIndex(1);

      //windowContentLayout->setContentsMargins(0, 0, 0, 16);

      QObject::connect(tabBar, &QTabBar::tabCloseRequested, tabBar, [tabBar](int index) {
        tabBar->removeTab(index);
      });
    }
#endif
#if 1
    {
      auto* groupBox = new QGroupBox(windowContent);
      groupBox->setTitle("Title of the GroupBox that can be very long");
      groupBox->setCheckable(true);
      groupBox->setAlignment(Qt::AlignRight);
      groupBox->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

      auto* radioGroup = new QButtonGroup(groupBox);

      auto* radio1 = new QRadioButton("Radio button 1");
      radio1->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      radioGroup->addButton(radio1);

      auto* radio2 = new QRadioButton("Radio button 2");
      radio2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
      radioGroup->addButton(radio2);

      auto* button1 = new QPushButton("Button 1");
      button1->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

      auto* button2 = new QPushButton("Button 2");
      button2->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

      radio1->setChecked(true);

      auto* vbox = new QVBoxLayout(groupBox);
      vbox->setContentsMargins(0, 0, 0, 0);
      groupBox->setLayout(vbox);
      //vbox->setSpacing(0);
      vbox->addWidget(radio1);
      vbox->addWidget(radio2);
      vbox->addWidget(button1);
      vbox->addWidget(button2);
      vbox->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));

      windowContentLayout->addWidget(groupBox);
    }
#endif
#if 0
    {
      // Widget to test the different bounding boxes returned by QFontMetrics.
      auto* customPaintWidget = new CustomPaintWidget(windowContent);
      customPaintWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      customPaintWidget->setMinimumSize(100, 100);
      windowContentLayout->addWidget(customPaintWidget);
    }
#endif
#if 0
    {
      const auto title = "Title of the QMessageBox";
      const auto text =
        R"(Lorem ipsum dolor sit amet, consectetur <a href="#">adipiscing elit</a>, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.)";
      const auto informativeText =
        R"(Vitae ut et dolorem eum. Rerum aut aut quis <a href="#">dolorum facere</a> quod veniam accusantium.
Accusamus quidem sed possimus aut consequatur soluta ut. Soluta ut enim quo reiciendis a tempora dolorum min…)";
      const auto detailedText =
        R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.
Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.
Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum)";
      const auto buttons = QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel;
      auto* qMessageBox = new QMessageBox(QMessageBox::Icon::Information, title, text, buttons, &owner);
      qMessageBox->setInformativeText(informativeText);
      qMessageBox->setDetailedText(detailedText);
      qMessageBox->show();

      QObject::connect(qMessageBox, &QMessageBox::finished, qMessageBox, []() { std::exit(0); });
    }
#endif
#if 0
    {
      const auto iconSize = QSize(128, 128);

      auto* label1 = new QLabel(windowContent);
      label1->setFixedSize(iconSize);
      const auto icon1 = QApplication::style()->standardIcon(QStyle::SP_MessageBoxCritical);
      label1->setPixmap(icon1.pixmap(iconSize.width()));
      windowContentLayout->addWidget(label1);

      auto* label2 = new QLabel(windowContent);
      label2->setFixedSize(iconSize);
      const auto icon2 = QApplication::style()->standardIcon(QStyle::SP_MessageBoxWarning);
      label2->setPixmap(icon2.pixmap(iconSize.width()));
      windowContentLayout->addWidget(label2);

      auto* label3 = new QLabel(windowContent);
      label3->setFixedSize(iconSize);
      const auto icon3 = QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation);
      label3->setPixmap(icon3.pixmap(iconSize.width()));
      windowContentLayout->addWidget(label3);

      auto* label4 = new QLabel(windowContent);
      label4->setFixedSize(iconSize);
      const auto icon4 = QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion);
      label4->setPixmap(icon4.pixmap(iconSize.width()));
      windowContentLayout->addWidget(label4);
    }
#endif

    windowContentLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    auto* centralWidgetContent = scrollArea;
    scrollArea->setWidget(scrollAreaContent);
    scrollArea->setWidgetResizable(true);
    owner.setCentralWidget(centralWidgetContent);
  }

  void setupUi_treeView() {
    auto* scrollArea = new QScrollArea(&owner);
    auto* scrollAreaContent = new QWidget(scrollArea);
    scrollAreaContent->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    windowContentLayout = new QVBoxLayout(scrollAreaContent);
    windowContent = scrollAreaContent;

    {
      auto* treeWidget = new QTreeWidget(windowContent);
      treeWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
      treeWidget->setAlternatingRowColors(false);
      treeWidget->setColumnCount(1);
      treeWidget->setHeaderHidden(true);
      treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
      treeWidget->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
      QlementineStyle::setAutoIconColorEnabled(treeWidget, false);

      for (auto i = 0; i < 3; ++i) {
        auto* root = new QTreeWidgetItem(treeWidget);
        root->setText(0, QString("Root %1").arg(i + 1));
        root->setIcon(0, QIcon(":/scene_object.svg"));
        root->setText(1, QString("Column 2 of Root %1").arg(i + 1));

        for (auto j = 0; j < 3; ++j) {
          auto* child = new QTreeWidgetItem(root);
          child->setText(0, QString("Child %1 of Root %2").arg(j).arg(i));
          child->setIcon(0, j == 2 ? QIcon(":/scene_light.svg") : QIcon(":/scene_object.svg"));
          child->setText(1, QString("Column 2 of Child %1 of Root %2").arg(j).arg(i));

          for (auto k = 0; k < 3; ++k) {
            auto* subChild = new QTreeWidgetItem(child);
            subChild->setText(0, QString("Child %1 of Child %2 of Root %3").arg(k).arg(j).arg(i));
            subChild->setIcon(0, QIcon(":/scene_material.svg"));
            subChild->setText(1, QString("Column 2 of Child %1 of Child %2 of Root %3").arg(k).arg(j).arg(i));
          }
        }
      }

      treeWidget->topLevelItem(0)->setSelected(true);
      windowContentLayout->addWidget(treeWidget);
    }

    {
      auto* listView = new QListWidget(windowContent);
      listView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
      //listView->setAlternatingRowColors(true);
      listView->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

      for (auto i = 0; i < 3; ++i) {
        auto* item = new QListWidgetItem(QIcon(":/refresh.svg"), QString("Item #%1 with very long text that can be elided").arg(i), listView);
        item->setFlags(item->flags() | Qt::ItemFlag::ItemIsUserCheckable);
        item->setCheckState(i % 2 ? Qt ::CheckState::Checked : Qt::CheckState::Unchecked);

        listView->addItem(item);
      }
      listView->item(0)->setSelected(true);
      windowContentLayout->addWidget(listView);
    }

    {
      constexpr auto columnCount = 3;
      constexpr auto rowCount = 3;

      auto* tableView = new QTableWidget(windowContent);
      tableView->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
      tableView->setColumnCount(columnCount);
      tableView->setRowCount(rowCount);
      QIcon icon(":/refresh.svg");
      auto* headerItem = new QTableWidgetItem(icon, "A veeeeeery long header label");
      tableView->setHorizontalHeaderItem(0, headerItem);
      tableView->setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);
      //tableView->verticalHeader()->hide();
      //tableView->horizontalHeader()->hide();
      //tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
      //tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeMode::Interactive);
      tableView->horizontalHeader()->setSectionResizeMode(columnCount - 1, QHeaderView::ResizeMode::Stretch);
      tableView->horizontalHeader()->setSortIndicatorShown(true);
      tableView->setShowGrid(false);

      for (auto i = 0; i < rowCount; ++i) {
        for (auto j = 0; j < columnCount; ++j) {
          auto* item = new QTableWidgetItem(QString("Row %1 / Column %2").arg(i + 1).arg(j + 1));
          item->setFlags(Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable);
          tableView->setItem(i, j, item);
        }
      }
      tableView->item(0, 0)->setSelected(true);

      windowContentLayout->addWidget(tableView);
    }

    windowContentLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    auto* centralWidgetContent = scrollArea;
    scrollArea->setWidget(scrollAreaContent);
    scrollArea->setWidgetResizable(true);
    owner.setCentralWidget(centralWidgetContent);

    owner.resize(400, 700);
  }

  void setupUi_expander() {
    auto* scrollArea = new QScrollArea(&owner);
    auto* scrollAreaContent = new QWidget(scrollArea);
    scrollAreaContent->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    windowContentLayout = new QVBoxLayout(scrollAreaContent);
    windowContent = scrollAreaContent;

    {
      auto* container = new CustomBgWidget(windowContent);
      auto* containerLayout = new QVBoxLayout(container);
      containerLayout->setContentsMargins(10, 10, 10, 10);
      container->setLayout(containerLayout);

      auto* expander = new Expander(container);
      auto* expanderContent = new CustomBgWidget(expander);
      expanderContent->bgColor = QColor{ 255, 127, 0 };
      expanderContent->customSizeHint = QSize{ 150, 100 };
      expanderContent->showBounds = true;
      expander->setContent(expanderContent);

      auto* checkBox = new QCheckBox("Expanded", container);
      QObject::connect(checkBox, &QCheckBox::toggled, &owner, [expander](bool checked) {
        expander->setExpanded(checked);
      });

      auto* button = new QPushButton("Increase content height", container);
      QObject::connect(button, &QPushButton::clicked, &owner, [expander, expanderContent]() {
        expanderContent->customSizeHint.rheight() += 20;
        expanderContent->updateGeometry();
      });
      containerLayout->addWidget(checkBox);
      containerLayout->addWidget(button);
      containerLayout->addWidget(expander);

      windowContentLayout->addWidget(container);
    }

    windowContentLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
    auto* centralWidgetContent = scrollArea;
    scrollArea->setWidget(scrollAreaContent);
    scrollArea->setWidgetResizable(true);
    owner.setCentralWidget(centralWidgetContent);
  }

  void setupUi_popover() {
    windowContent = new QWidget(&owner);
    windowContentLayout = new QVBoxLayout(windowContent);

#if 0
    auto* popoverButton = new PopoverButton("Open popup", windowContent);
    popoverButton->popover()->setPreferredPosition(Popover::Position::Top);
    popoverButton->popover()->setPreferredAlignment(Popover::Alignment::Center);
    popoverButton->popover()->setVerticalSpacing(10);
    auto* dummyWidget = new CustomBgWidget();
    dummyWidget->bgColor = Qt::transparent;
    dummyWidget->customSizeHint = QSize{ 150, 150 };
    popoverButton->setPopoverContentWidget(dummyWidget);
    windowContentLayout->addWidget(popoverButton);
    owner.setCentralWidget(windowContent);
    return;
#endif

    auto* anchorLabel = new QLabel("The popover positions itself relatively to this widget:", windowContent);
    anchorLabel->setWordWrap(true);
    windowContentLayout->addWidget(anchorLabel);

    auto* anchorWidget = new CustomBgWidget(&owner);
    anchorWidget->showBounds = false;
    anchorWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    anchorWidget->customSizeHint = QSize(100, 100);
    anchorWidget->bgColor = Qt::blue;
    windowContentLayout->addWidget(anchorWidget, 1, Qt::AlignCenter);

    auto* popoverCheckBox = new QCheckBox("Popup is opened", windowContent);
    windowContentLayout->addWidget(popoverCheckBox, 0, Qt::AlignBottom);

    auto* positionLabel = new QLabel("Position:", windowContent);
    windowContentLayout->addWidget(positionLabel);

    auto* comboBoxPosition = new QComboBox(&owner);
    comboBoxPosition->addItem("Left", static_cast<int>(Popover::Position::Left));
    comboBoxPosition->addItem("Right", static_cast<int>(Popover::Position::Right));
    comboBoxPosition->addItem("Top", static_cast<int>(Popover::Position::Top));
    comboBoxPosition->addItem("Bottom", static_cast<int>(Popover::Position::Bottom));
    windowContentLayout->addWidget(comboBoxPosition);

    auto* alignmentLabel = new QLabel("Alignment:", windowContent);
    windowContentLayout->addWidget(alignmentLabel);

    auto* comboBoxAlignment = new QComboBox(&owner);
    comboBoxAlignment->addItem("Begin", static_cast<int>(Popover::Alignment::Begin));
    comboBoxAlignment->addItem("Center", static_cast<int>(Popover::Alignment::Center));
    comboBoxAlignment->addItem("End", static_cast<int>(Popover::Alignment::End));
    windowContentLayout->addWidget(comboBoxAlignment);

    auto* popover = new Popover(anchorWidget);
    popover->setPadding({ 0, 0, 0, 0 });
    popover->setHorizontalSpacing(0);
    popover->setVerticalSpacing(0);

    auto* hSpacingLabel = new QLabel("Horizontal Spacing:", windowContent);
    windowContentLayout->addWidget(hSpacingLabel);

    auto* hSpacingSpinBox = new QSpinBox(&owner);
    hSpacingSpinBox->setRange(-100, 100);
    windowContentLayout->addWidget(hSpacingSpinBox);

    auto* vSpacingLabel = new QLabel("Vertical Spacing:", windowContent);
    windowContentLayout->addWidget(vSpacingLabel);

    auto* vSpacingSpinBox = new QSpinBox(&owner);
    vSpacingSpinBox->setRange(-100, 100);
    windowContentLayout->addWidget(vSpacingSpinBox);

#if 1
    auto* popoverContent = new QWidget();
    auto* popoverContentLayout = new QVBoxLayout(popoverContent);
    popoverContentLayout->setContentsMargins(0, 0, 0, 0);
    {
      for (auto i = 0; i < 3; ++i) {
        auto* btn = new QPushButton(QString("QPushButton %1").arg(i + 1), popoverContent);
        popoverContentLayout->addWidget(btn);
      }
    }
#else
    auto* popoverContent = new CustomBgWidget();
    popoverContent->showBounds = true;
    popoverContent->customSizeHint = QSize(100, 200);

#endif
    popover->setContentWidget(popoverContent);
    popover->setAnchorWidget(anchorWidget);

    // Synchronize opened state.
    popoverCheckBox->setChecked(false);
    QObject::connect(popoverCheckBox, &QAbstractButton::clicked, &owner, [popover](bool checked) {
      popover->setOpened(checked);
    });
    QObject::connect(popover, &Popover::openedChanged, &owner, [popover, popoverCheckBox]() {
      QSignalBlocker b{ popoverCheckBox };
      popoverCheckBox->setChecked(popover->isOpened());
    });

    // Synchronize position.
    comboBoxPosition->setCurrentIndex(comboBoxPosition->findData(static_cast<int>(popover->preferredPosition())));
    QObject::connect(comboBoxPosition, qOverload<int>(&QComboBox::currentIndexChanged), &owner, [comboBoxPosition, popover](int index) {
      const auto pos = static_cast<Popover::Position>(comboBoxPosition->itemData(index).toInt());
      popover->setPreferredPosition(pos);
    });
    QObject::connect(popover, &Popover::preferredPositionChanged, &owner, [popover, comboBoxPosition]() {
      QSignalBlocker b{ comboBoxPosition };
      comboBoxPosition->setCurrentIndex(comboBoxPosition->findData(static_cast<int>(popover->preferredPosition())));
    });

    // Synchronize alignment.
    comboBoxAlignment->setCurrentIndex(comboBoxAlignment->findData(static_cast<int>(popover->preferredAlignment())));
    QObject::connect(comboBoxAlignment, qOverload<int>(&QComboBox::currentIndexChanged), &owner, [comboBoxAlignment, popover](int index) {
      const auto align = static_cast<Popover::Alignment>(comboBoxAlignment->itemData(index).toInt());
      popover->setPreferredAlignment(align);
    });
    QObject::connect(popover, &Popover::preferredAlignmentChanged, &owner, [popover, comboBoxAlignment]() {
      QSignalBlocker b{ comboBoxAlignment };
      comboBoxAlignment->setCurrentIndex(comboBoxAlignment->findData(static_cast<int>(popover->preferredAlignment())));
    });

    // Synchronize horizontal spacing.
    hSpacingSpinBox->setValue(popover->horizontalSpacing());
    QObject::connect(hSpacingSpinBox, qOverload<int>(&QSpinBox::valueChanged), &owner, [popover](int v) {
      popover->setHorizontalSpacing(v);
    });
    QObject::connect(popover, &Popover::horizontalSpacingChanged, &owner, [hSpacingSpinBox, popover]() {
      hSpacingSpinBox->setValue(popover->horizontalSpacing());
    });

    // Synchronize vertical spacing.
    vSpacingSpinBox->setValue(popover->verticalSpacing());
    QObject::connect(vSpacingSpinBox, qOverload<int>(&QSpinBox::valueChanged), &owner, [popover](int v) {
      popover->setVerticalSpacing(v);
    });
    QObject::connect(popover, &Popover::verticalSpacingChanged, &owner, [vSpacingSpinBox, popover]() {
      vSpacingSpinBox->setValue(popover->verticalSpacing());
    });

    owner.setCentralWidget(windowContent);
  }

  void setupUi_navigationBar() {
    windowContent = new QWidget(&owner);
    windowContentLayout = new QVBoxLayout(windowContent);

    const QIcon dummyIcon(":/refresh.svg");

    auto* navBar = new NavigationBar(windowContent);
    for (auto i = 0; i < 3; ++i) {
      navBar->addItem(QString("Item %1").arg(i), dummyIcon, QString("%1").arg((i + 1) * 10));
    }

    auto* segmCtrl = new SegmentedControl(windowContent);
    for (auto i = 0; i < 3; ++i) {
      segmCtrl->addItem(QString("Item %1").arg(i), dummyIcon, QString("%1").arg((i + 1) * 10));
    }

    windowContentLayout->addWidget(navBar);
    windowContentLayout->addWidget(segmCtrl);
    owner.setCentralWidget(windowContent);
  }

  void setupUi_switch() {
    windowContent = new QWidget(&owner);
    windowContentLayout = new QVBoxLayout(windowContent);

    const QIcon dummyIcon(":/refresh.svg");

    auto* switchWidget = new Switch(windowContent);
    switchWidget->setText("Label of the Switch");
    switchWidget->setIcon(dummyIcon);
    switchWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);

    windowContentLayout->addWidget(switchWidget);
    owner.setCentralWidget(windowContent);
  }

  static QPixmap getInputPixmap() {
    constexpr auto w = 100;
    QPixmap pixmap(w, w);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::red);
    p.drawEllipse(QRect(0, 0, w, w));
    return pixmap;
  }

  static QPixmap getInputWithShadow(const QPixmap& input, double blurRadius) {
    const auto shadow = qlementine::getDropShadowPixmap(input, blurRadius);
    QPixmap result(shadow.size());
    result.fill(Qt::transparent);

    QPainter p(&result);
    p.drawPixmap((result.width() - shadow.width()) / 2, (result.height() - shadow.height()) / 2, shadow);
    p.drawPixmap((result.width() - input.width()) / 2, (result.height() - input.height()) / 2, input);

    return result;
  }

  void setupUi_blur() {
    //constexpr auto extendImage = true;
    constexpr auto initialBlur = 1;
    windowContent = new QWidget(&owner);
    windowContentLayout = new QHBoxLayout(windowContent);

    const auto inputPixmap = getInputPixmap();
    auto* labelBefore = new QLabel(windowContent);
    labelBefore->setPixmap(inputPixmap);
    labelBefore->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    windowContentLayout->addWidget(labelBefore, 0, Qt::AlignRight);

    const auto outputPixmap = getInputWithShadow(inputPixmap, initialBlur);
    auto* labelAfter = new QLabel(windowContent);
    labelAfter->setPixmap(outputPixmap);
    labelAfter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    windowContentLayout->addWidget(labelAfter, 0, Qt::AlignLeft);

    auto* slider = new QSlider(windowContent);
    slider->setOrientation(Qt::Horizontal);
    slider->setRange(0, 10);
    slider->setValue(initialBlur);
    slider->setMinimumWidth(200);
    QObject::connect(slider, &QSlider::valueChanged, &owner, [inputPixmap, labelAfter](int value) {
      const auto outputPixmap = getInputWithShadow(inputPixmap, value);
      labelAfter->setPixmap(outputPixmap);
    });
    windowContentLayout->addWidget(slider, 0, Qt::AlignLeft);

    owner.setCentralWidget(windowContent);
  }

  void setupUi_focus() {
    windowContent = new QWidget(&owner);
    windowContentLayout = new QVBoxLayout(windowContent);

    auto* button1 = new QPushButton("Button 1");
    button1->setObjectName("button1");
    windowContentLayout->addWidget(button1);

    auto* button2 = new QPushButton("Button 2");
    button2->setObjectName("button2");
    windowContentLayout->addWidget(button2);

    owner.setCentralWidget(windowContent);
  }

  void setup_badge() {
    windowContent = new QWidget(&owner);
    windowContentLayout = new QHBoxLayout(windowContent);

    windowContentLayout->addWidget(new StatusBadgeWidget(StatusBadge::Info, StatusBadgeSize::Medium, windowContent));
    windowContentLayout->addWidget(new StatusBadgeWidget(StatusBadge::Error, StatusBadgeSize::Medium, windowContent));
    windowContentLayout->addWidget(new StatusBadgeWidget(StatusBadge::Success, StatusBadgeSize::Medium, windowContent));
    windowContentLayout->addWidget(new StatusBadgeWidget(StatusBadge::Warning, StatusBadgeSize::Medium, windowContent));
    windowContentLayout->addWidget(new StatusBadgeWidget(StatusBadge::Info, StatusBadgeSize::Small, windowContent));
    windowContentLayout->addWidget(new StatusBadgeWidget(StatusBadge::Error, StatusBadgeSize::Small, windowContent));
    windowContentLayout->addWidget(new StatusBadgeWidget(StatusBadge::Success, StatusBadgeSize::Small, windowContent));
    windowContentLayout->addWidget(new StatusBadgeWidget(StatusBadge::Warning, StatusBadgeSize::Small, windowContent));

    owner.setCentralWidget(windowContent);
  }
#if 0
  void setup_flowLayout() {
    windowContent = new QWidget(&owner);
    windowContent->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    windowContentLayout = new QVBoxLayout(windowContent);

    auto* container = new CustomBgWidget(windowContent);
    container->setMinimumSize(0, 0);
    container->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    container->customSizeHint = QSize(0, 0);
    container->bgColor = QColor(255, 0, 0, 32);
    container->showBounds = true;
    auto* containerLayout = new qlementine::FlowLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    container->setLayout(containerLayout);
    windowContentLayout->addWidget(container);

    auto* addButton = new QPushButton("Add", windowContent);
    windowContentLayout->addWidget(addButton);
    auto* removeButton = new QPushButton("Remove", windowContent);
    windowContentLayout->addWidget(removeButton);

    QObject::connect(addButton, &QPushButton::clicked, addButton, [container, containerLayout]() {
      auto* element = new QPushButton(QString::number(containerLayout->count()), container);
      containerLayout->addWidget(element);
    });
    QObject::connect(removeButton, &QPushButton::clicked, removeButton, [containerLayout]() {
      if (auto* layoutItem = containerLayout->takeAt(0)) {
        if (auto* w = layoutItem->widget()) {
          delete w;
        }
        delete layoutItem;
      }
    });
    owner.setCentralWidget(windowContent);
  }
#endif

  void setup_specialProgressBar() {
    windowContent = new QWidget(&owner);
    windowContentLayout = new QVBoxLayout(windowContent);

    //{
    //  auto* progressBar = new QProgressBar(windowContent);
    //  progressBar->setTextVisible(false);
    //  progressBar->setRange(0, 100);
    //  progressBar->setValue(30);
    //  progressBar->setInvertedAppearance(true);
    //  windowContentLayout->addWidget(progressBar);
    //}
    {
      auto* progressBar = new QProgressBar(windowContent);
      progressBar->setTextVisible(false);
      progressBar->setRange(0, 0);
      windowContentLayout->addWidget(progressBar);
    }

    owner.setCentralWidget(windowContent);
  }

  void setup_lineEdit() {
    windowContent = new QWidget(&owner);
    windowContentLayout = new QVBoxLayout(windowContent);

    const QIcon dummyIcon(":/refresh.svg");

    auto* lineEdit = new LineEdit(windowContent);
    lineEdit->setText("Label of the Switch");
    lineEdit->setIcon(dummyIcon);
    lineEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Fixed);
    lineEdit->setClearButtonEnabled(true);
    windowContentLayout->addWidget(lineEdit);

    auto* comboBoxStatus = new QComboBox(&owner);
    comboBoxStatus->addItem("Default", static_cast<int>(Status::Default));
    comboBoxStatus->addItem("Error", static_cast<int>(Status::Error));
    comboBoxStatus->addItem("Warning", static_cast<int>(Status::Warning));
    comboBoxStatus->addItem("Success", static_cast<int>(Status::Success));
    windowContentLayout->addWidget(comboBoxStatus);
    QObject::connect(comboBoxStatus, qOverload<int>(&QComboBox::currentIndexChanged), &owner, [comboBoxStatus, lineEdit](int index) {
      const auto value = static_cast<Status>(comboBoxStatus->itemData(index).toInt());
      lineEdit->setStatus(value);
    });

    owner.setCentralWidget(windowContent);
  }

  SandboxWindow& owner;
  QString lastJsonThemePath;
  QPointer<QlementineStyle> qlementineStyle;

  QWidget* windowContent{ nullptr };
  QBoxLayout* windowContentLayout{ nullptr };
  QToolBar* toolbar{ nullptr };

  QAction* reloadJsonAction{ nullptr };
  QFileSystemWatcher fileWatcher;
};

SandboxWindow::SandboxWindow(QWidget* parent)
  : QMainWindow(parent)
  , _impl(new Impl(*this)) {
  setWindowIcon(QIcon(QStringLiteral(":/qlementine_icon.ico")));

  //_impl->setupMenuBar();
  _impl->setupShortcuts();
  //_impl->setupUi_basicWidgets();
  //_impl->setupUi_popover();
  //_impl->setupUi_focus();
  //_impl->setupUi_treeView();
  //_impl->setupUi_expander();
  //_impl->setup_badge();
  //_impl->setup_flowLayout();
  //_impl->setup_specialProgressBar();
  _impl->setup_lineEdit();
}

SandboxWindow::~SandboxWindow() = default;

void SandboxWindow::setCustomStyle(QlementineStyle* style) {
  _impl->qlementineStyle = style;
  _impl->lastJsonThemePath = QStringLiteral(":/light.json");
}
} // namespace oclero::qlementine::sandbox
