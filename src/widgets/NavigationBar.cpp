#include <oclero/qlementine/widgets/NavigationBar.hpp>

#include <oclero/qlementine/style/QlementineStyle.hpp>
#include <oclero/qlementine/utils/ImageUtils.hpp>

namespace oclero::qlementine {
const QColor& NavigationBar::getBgColor(const Theme& theme) const {
	return theme.backgroundColorMain1;
}

const QColor& NavigationBar::getItemBgColor(MouseState mouse, const Theme& theme) const {
	switch (mouse) {
		case MouseState::Hovered:
			return theme.adaptativeColor4;
		case MouseState::Pressed:
			return theme.adaptativeColor5;
		default:
			return theme.adaptativeColorTransparent;
	}
}

const QColor& NavigationBar::getItemFgColor(MouseState mouse, bool /*selected*/, const Theme& theme) const {
	switch (mouse) {
		case MouseState::Disabled:
			return theme.neutralColorDisabled;
		default:
			return theme.neutralColor;
	}
}

const QColor& NavigationBar::getItemBadgeBgColor(MouseState mouse, bool /*selected*/, const Theme& theme) const {
	return mouse == MouseState::Disabled ? theme.adaptativeColor1 : theme.adaptativeColor2;
}

const QColor& NavigationBar::getItemBadgeFgColor(MouseState mouse, bool /*selected*/, const Theme& theme) const {
	if (mouse == MouseState::Disabled)
		return theme.neutralColorDisabled;
	else
		return theme.neutralColor;
}

QMargins NavigationBar::getItemPadding() const {
	const auto* style = this->style();
	const auto left = style->pixelMetric(QStyle::PM_LayoutLeftMargin);
	const auto top = style->pixelMetric(QStyle::PM_LayoutTopMargin) / 2;
	const auto right = style->pixelMetric(QStyle::PM_LayoutRightMargin);
	const auto bottom = style->pixelMetric(QStyle::PM_LayoutBottomMargin) / 2;
	return QMargins{ left, top, right, bottom };
}

double NavigationBar::getItemRadius() const {
	return 0.0;
}

QMargins NavigationBar::getPadding() const {
	return QMargins{ 0, 0, 0, 0 };
}

int NavigationBar::getSpacing() const {
	return 0.0;
}

int NavigationBar::getItemMinimumHeight() const {
	const auto* qlementineStyle = qobject_cast<const QlementineStyle*>(style());
	return qlementineStyle ? qlementineStyle->theme().controlHeightMedium * 2 : 48;
}

void NavigationBar::drawCurrentItemIndicator(QPainter& p) const {
	const auto currentItemRect = getAnimatedCurrentItemRect();
	const auto* qlementineStyle = qobject_cast<const QlementineStyle*>(style());
	const auto thickness = qlementineStyle ? qlementineStyle->theme().borderWidth * 3 : 3;
	const auto rect = QRect{ currentItemRect.x(), currentItemRect.y() + currentItemRect.height() - thickness, currentItemRect.width(), thickness };
	const auto& color = getCurrentItemIndicatorColor();
	p.setPen(Qt::NoPen);
	p.fillRect(rect, color);
}
QFont NavigationBar::labelFont() const {
	const auto* qlementineStyle = qobject_cast<const QlementineStyle*>(style());
	const auto& biggerFont = qlementineStyle ? qlementineStyle->theme().fontForTextRole(qlementine::Theme::TextRole::H5) : this->font();
	return biggerFont;
}
} // namespace oclero::qlementine
