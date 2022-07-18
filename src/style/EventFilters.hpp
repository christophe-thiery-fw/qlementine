#pragma once

#include <oclero/qlementine/style/QlementineStyle.hpp>
#include <oclero/qlementine/animation/WidgetAnimationManager.hpp>

namespace oclero::qlementine {
class LineEditButtonEventFilter : public QObject {
public:
	LineEditButtonEventFilter(QlementineStyle& style, WidgetAnimationManager& animManager, QToolButton* button);

	bool eventFilter(QObject* watchedObject, QEvent* evt) override;

private:
	QlementineStyle& _style;
	WidgetAnimationManager& _animManager;
	QToolButton* _button{ nullptr };
};

class CommandLinkButtonPaintEventFilter : public QObject {
public:
	CommandLinkButtonPaintEventFilter(QlementineStyle& style, WidgetAnimationManager& animManager, QCommandLinkButton* button);

	bool eventFilter(QObject* watchedObject, QEvent* evt) override;

private:
	QlementineStyle& _style;
	WidgetAnimationManager& _animManager;
	QCommandLinkButton* _button{ nullptr };
};

class MouseWheelBlockerEventFilter : public QObject {
public:
	MouseWheelBlockerEventFilter(QWidget* widget);

	bool eventFilter(QObject* watchedObject, QEvent* evt) override;

private:
	QWidget* _widget{ nullptr };
};

class TabBarEventFilter : public QObject {
public:
	TabBarEventFilter(QTabBar* tabBar);

	bool eventFilter(QObject* watchedObject, QEvent* evt) override;

private:
	QTabBar* _tabBar{ nullptr };
	QToolButton* _leftButton{ nullptr };
	QToolButton* _rightButton{ nullptr };
};

class MenuEventFilter : public QObject {
public:
	MenuEventFilter(QMenu* menu);

	bool eventFilter(QObject* watchedObject, QEvent* evt) override;

private:
	QMenu* _menu{ nullptr };
};

class ComboboxItemViewFilter : public QObject {
public:
	ComboboxItemViewFilter(QAbstractItemView* view);

	bool eventFilter(QObject* watchedObject, QEvent* evt) override;

private:
	QAbstractItemView* _view{ nullptr };
};
} // namespace oclero::qlementine
