#include "hex_view_widget.h"

#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include <QtGui/QFontMetrics>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>
#include <QtWidgets/QScrollBar>

#include <algorithm>

namespace
{
    constexpr int kBytesPerRow  = 16;
    constexpr int kCharCount    = 1 + 8 + 1 + 1 + 1 + 16 * 3 + 1 + 1 + 16 + 1;
    constexpr int kAddressChars = 8;
} // namespace

HexViewWidget::HexViewWidget(QWidget* parent)
    : QAbstractScrollArea(parent)
    , m_bytes()
    , m_highlightRange(std::nullopt)
    , m_selectionPart(SelectionPart::None)
    , m_selectionAnchor(0U)
    , m_selectionCurrent(0U)
    , m_isSelecting(false)
    , m_charWidth(0)
    , m_lineHeight(0)
    , m_headerHeight(0)
    , m_addrX(0)
    , m_hexX(0)
    , m_asciiX(0)
    , m_totalWidth(0)
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setFamily("Consolas");
    font.setStyleHint(QFont::TypeWriter);
    font.setFixedPitch(true);
    font.setPointSize(10);
    setFont(font);

    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::StrongFocus);
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    viewport()->setAutoFillBackground(true);

    updateMetrics();
    updateScrollBar();
}

void HexViewWidget::clear()
{
    m_bytes.clear();
    m_highlightRange.reset();
    m_selectionPart    = SelectionPart::None;
    m_selectionAnchor  = 0U;
    m_selectionCurrent = 0U;
    m_isSelecting      = false;
    updateScrollBar();
    viewport()->update();
}

void HexViewWidget::setBytes(const std::string& bytes)
{
    m_bytes            = bytes;
    m_selectionPart    = SelectionPart::None;
    m_selectionAnchor  = 0U;
    m_selectionCurrent = 0U;
    m_isSelecting      = false;
    verticalScrollBar()->setValue(0);
    updateScrollBar();
    viewport()->update();
}

void HexViewWidget::setHighlightRange(const std::size_t offset, const std::size_t encodedLength, const std::size_t valueLength)
{
    if (encodedLength == 0U || offset >= m_bytes.size())
    {
        m_highlightRange.reset();
    }
    else
    {
        m_highlightRange = HighlightRange{offset, encodedLength, valueLength};
    }
    viewport()->update();
}

void HexViewWidget::scrollToOffset(const std::size_t offset)
{
    if (m_bytes.empty())
    {
        return;
    }

    const std::size_t boundedOffset = std::min(offset, m_bytes.size() - 1U);
    const int         row           = static_cast<int>(boundedOffset / static_cast<std::size_t>(kBytesPerRow));
    ensureVisibleRow(row);
}

void HexViewWidget::contextMenuEvent(QContextMenuEvent* event)
{
    QMenu    menu(this);
    QAction* copyAction = menu.addAction(tr("复制"));
    copyAction->setEnabled(selectionRange().isValid());
    QAction* selectedAction = menu.exec(event->globalPos());
    if (selectedAction == copyAction)
    {
        copySelection();
    }
}

void HexViewWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->matches(QKeySequence::Copy))
    {
        copySelection();
        event->accept();
        return;
    }
    QAbstractScrollArea::keyPressEvent(event);
}

void HexViewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (!m_isSelecting || m_selectionPart == SelectionPart::None)
    {
        QAbstractScrollArea::mouseMoveEvent(event);
        return;
    }

    m_selectionCurrent = positionFromPoint(event->pos(), m_selectionPart);
    if (m_selectionPart == SelectionPart::Address)
    {
        ensureVisibleRow(static_cast<int>(m_selectionCurrent));
    }
    else
    {
        ensureVisibleRow(static_cast<int>(m_selectionCurrent / static_cast<std::size_t>(kBytesPerRow)));
    }
    viewport()->update();
}

void HexViewWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton)
    {
        QAbstractScrollArea::mousePressEvent(event);
        return;
    }

    setFocus(Qt::MouseFocusReason);
    const SelectionPart part = selectionPartAt(event->pos().x());
    if (part == SelectionPart::None || rowFromY(event->pos().y()) < 0)
    {
        m_selectionPart = SelectionPart::None;
        m_isSelecting   = false;
        viewport()->update();
        return;
    }

    m_selectionPart    = part;
    m_selectionAnchor  = positionFromPoint(event->pos(), part);
    m_selectionCurrent = m_selectionAnchor;
    m_isSelecting      = true;
    viewport()->update();
}

void HexViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isSelecting = false;
        viewport()->update();
        return;
    }
    QAbstractScrollArea::mouseReleaseEvent(event);
}

void HexViewWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(viewport());
    painter.fillRect(viewport()->rect(), Qt::white);
    painter.setFont(font());

    drawHeader(painter);
    drawRows(painter);
}

void HexViewWidget::resizeEvent(QResizeEvent* event)
{
    QAbstractScrollArea::resizeEvent(event);
    updateScrollBar();
}

void HexViewWidget::copySelection() const
{
    const QString text = selectedText();
    if (text.isEmpty())
    {
        return;
    }
    QGuiApplication::clipboard()->setText(text);
}

void HexViewWidget::ensureVisibleRow(const int row)
{
    const int topRow    = verticalScrollBar()->value();
    const int bottomRow = topRow + visibleRowCount() - 1;

    if (row < topRow)
    {
        verticalScrollBar()->setValue(row);
    }
    else if (row > bottomRow)
    {
        verticalScrollBar()->setValue(row - visibleRowCount() + 1);
    }
}

void HexViewWidget::updateMetrics()
{
    const QFontMetrics metrics(font());
    m_charWidth    = std::max(metrics.horizontalAdvance(QLatin1Char('0')), 1);
    m_lineHeight   = metrics.height() + 4;
    m_headerHeight = m_lineHeight + 4;
    m_addrX        = m_charWidth;
    m_hexX         = m_addrX + 11 * m_charWidth;
    m_asciiX       = m_hexX + 49 * m_charWidth + m_charWidth;
    m_totalWidth   = kCharCount * m_charWidth;

    setMinimumWidth(m_totalWidth + verticalScrollBar()->sizeHint().width() + 8);
}

void HexViewWidget::updateScrollBar()
{
    const int rows        = rowCount();
    const int visibleRows = visibleRowCount();
    verticalScrollBar()->setPageStep(visibleRows);
    verticalScrollBar()->setSingleStep(1);
    verticalScrollBar()->setRange(0, std::max(rows - visibleRows, 0));
}

int HexViewWidget::rowCount() const
{
    if (m_bytes.empty())
    {
        return 0;
    }
    return static_cast<int>((m_bytes.size() + static_cast<std::size_t>(kBytesPerRow - 1)) / static_cast<std::size_t>(kBytesPerRow));
}

int HexViewWidget::visibleRowCount() const
{
    const int contentHeight = std::max(viewport()->height() - m_headerHeight, 0);
    return std::max(contentHeight / std::max(m_lineHeight, 1), 1);
}

HexViewWidget::ByteColor HexViewWidget::colorForByte(const std::size_t index) const
{
    const ByteColor base{Qt::white, QColor(120, 120, 120)};
    if (!m_highlightRange.has_value())
    {
        return base;
    }

    const HighlightRange& range = m_highlightRange.value();
    const std::size_t     end   = std::min(range.offset + range.encodedLength, m_bytes.size());
    if (index < range.offset || index >= end)
    {
        return base;
    }

    const std::size_t headerLength      = range.encodedLength > range.valueLength ? range.encodedLength - range.valueLength : 0U;
    const std::size_t tagLength         = headerLength > 0U ? 1U : 0U;
    const std::size_t lengthFieldLength = headerLength > tagLength ? headerLength - tagLength : 0U;

    if (index < range.offset + tagLength)
    {
        return ByteColor{QColor(255, 204, 128), QColor(102, 60, 0)};
    }
    if (index < range.offset + tagLength + lengthFieldLength)
    {
        return ByteColor{QColor(144, 202, 249), QColor(13, 71, 161)};
    }
    return ByteColor{QColor(200, 230, 201), QColor(27, 94, 32)};
}

QString HexViewWidget::buildAsciiChar(const unsigned char value) const
{
    if (value >= 32U && value <= 126U)
    {
        return QString(QChar(static_cast<char>(value)));
    }
    return ".";
}

int HexViewWidget::rowFromY(const int y) const
{
    if (y < m_headerHeight)
    {
        return -1;
    }

    const int row = verticalScrollBar()->value() + (y - m_headerHeight) / std::max(m_lineHeight, 1);
    return row >= rowCount() ? -1 : row;
}

bool HexViewWidget::isByteSelected(const std::size_t index, const SelectionPart part) const
{
    const SelectionRange range = selectionRange();
    return range.isValid() && range.part == part && index >= range.start && index <= range.end;
}

bool HexViewWidget::isRowSelected(const int row) const
{
    const SelectionRange range = selectionRange();
    return range.isValid() && range.part == SelectionPart::Address && row >= static_cast<int>(range.start) && row <= static_cast<int>(range.end);
}

HexViewWidget::SelectionPart HexViewWidget::selectionPartAt(const int x) const
{
    if (x >= m_addrX && x < m_addrX + kAddressChars * m_charWidth)
    {
        return SelectionPart::Address;
    }
    if (x >= m_hexX && x < m_asciiX - 2 * m_charWidth)
    {
        return SelectionPart::Hex;
    }
    if (x >= m_asciiX && x < m_asciiX + kBytesPerRow * m_charWidth)
    {
        return SelectionPart::Ascii;
    }
    return SelectionPart::None;
}

HexViewWidget::SelectionRange HexViewWidget::selectionRange() const
{
    if (m_selectionPart == SelectionPart::None)
    {
        return {};
    }

    SelectionRange range;
    range.part  = m_selectionPart;
    range.start = std::min(m_selectionAnchor, m_selectionCurrent);
    range.end   = std::max(m_selectionAnchor, m_selectionCurrent);

    if (range.part == SelectionPart::Address)
    {
        const std::size_t maxRow = rowCount() > 0 ? static_cast<std::size_t>(rowCount() - 1) : 0U;
        range.start              = std::min(range.start, maxRow);
        range.end                = std::min(range.end, maxRow);
    }
    else if (!m_bytes.empty())
    {
        const std::size_t maxIndex = m_bytes.size() - 1U;
        range.start                = std::min(range.start, maxIndex);
        range.end                  = std::min(range.end, maxIndex);
    }
    else
    {
        range.part = SelectionPart::None;
    }
    return range;
}

std::size_t HexViewWidget::positionFromPoint(const QPoint& pos, const SelectionPart part) const
{
    const int row = std::max(rowFromY(pos.y()), 0);
    if (part == SelectionPart::Address)
    {
        return static_cast<std::size_t>(row);
    }

    const int relativeX = part == SelectionPart::Hex ? pos.x() - m_hexX : pos.x() - m_asciiX;
    int       column    = 0;
    if (part == SelectionPart::Hex)
    {
        column = relativeX <= 0 ? 0 : relativeX / std::max(3 * m_charWidth, 1);
    }
    else
    {
        column = relativeX <= 0 ? 0 : relativeX / std::max(m_charWidth, 1);
    }
    column = std::clamp(column, 0, kBytesPerRow - 1);

    const std::size_t index = static_cast<std::size_t>(row) * static_cast<std::size_t>(kBytesPerRow) + static_cast<std::size_t>(column);
    return m_bytes.empty() ? 0U : std::min(index, m_bytes.size() - 1U);
}

QString HexViewWidget::selectedText() const
{
    const SelectionRange range = selectionRange();
    if (!range.isValid())
    {
        return {};
    }

    QStringList lines;
    if (range.part == SelectionPart::Address)
    {
        for (std::size_t row = range.start; row <= range.end; ++row)
        {
            const std::size_t offset = row * static_cast<std::size_t>(kBytesPerRow);
            lines.append(QString("%1").arg(static_cast<qulonglong>(offset), 8, 16, QLatin1Char('0')).toUpper());
        }
        return lines.join('\n');
    }

    for (std::size_t row = range.start / static_cast<std::size_t>(kBytesPerRow); row <= range.end / static_cast<std::size_t>(kBytesPerRow); ++row)
    {
        const std::size_t rowStart = row * static_cast<std::size_t>(kBytesPerRow);
        const std::size_t rowEnd   = std::min(rowStart + static_cast<std::size_t>(kBytesPerRow), m_bytes.size());
        const std::size_t start    = std::max(range.start, rowStart);
        const std::size_t end      = std::min(range.end + 1U, rowEnd);
        QString           line;

        for (std::size_t index = start; index < end; ++index)
        {
            const unsigned char byteValue = static_cast<unsigned char>(m_bytes[index]);
            if (range.part == SelectionPart::Hex)
            {
                if (!line.isEmpty())
                {
                    line += QLatin1Char(' ');
                }
                line += QString("%1").arg(static_cast<int>(byteValue), 2, 16, QLatin1Char('0')).toUpper();
            }
            else
            {
                line += buildAsciiChar(byteValue);
            }
        }
        lines.append(line);
    }
    return lines.join('\n');
}

void HexViewWidget::drawHeader(QPainter& painter)
{
    painter.fillRect(QRect(0, 0, viewport()->width(), m_headerHeight), QColor(236, 239, 241));
    painter.setPen(QColor(55, 71, 79));

    const int baseline = m_headerHeight - 6;
    painter.drawText(m_addrX, baseline, "Addr");
    for (int column = 0; column < kBytesPerRow; ++column)
    {
        const int x = m_hexX + column * 3 * m_charWidth;
        painter.drawText(x, baseline, QString("%1").arg(column, 2, 16, QLatin1Char('0')).toUpper());
    }
    painter.drawText(m_asciiX, baseline, "ASCII");

    painter.setPen(QColor(158, 158, 158));
    painter.drawText(m_addrX + 9 * m_charWidth, baseline, "|");
    painter.drawText(m_asciiX - 2 * m_charWidth, baseline, "|");
}

void HexViewWidget::drawRows(QPainter& painter)
{
    const int firstRow    = verticalScrollBar()->value();
    const int rows        = rowCount();
    const int visibleRows = visibleRowCount();
    const int lastRow     = std::min(firstRow + visibleRows + 1, rows);

    for (int row = firstRow; row < lastRow; ++row)
    {
        const int         y               = m_headerHeight + (row - firstRow) * m_lineHeight;
        const int         baseline        = y + m_lineHeight - 6;
        const std::size_t rowOffset       = static_cast<std::size_t>(row) * static_cast<std::size_t>(kBytesPerRow);
        const bool        addressSelected = isRowSelected(row);

        if (addressSelected)
        {
            painter.fillRect(QRect(m_addrX - 2, y + 1, 8 * m_charWidth + 4, m_lineHeight - 2), QColor(207, 216, 220));
        }

        painter.setPen(QColor(96, 125, 139));
        painter.drawText(m_addrX, baseline, QString("%1").arg(static_cast<qulonglong>(rowOffset), 8, 16, QLatin1Char('0')).toUpper());

        painter.setPen(QColor(158, 158, 158));
        painter.drawText(m_addrX + 9 * m_charWidth, baseline, "|");
        painter.drawText(m_asciiX - 2 * m_charWidth, baseline, "|");

        for (int column = 0; column < kBytesPerRow; ++column)
        {
            const std::size_t byteIndex = rowOffset + static_cast<std::size_t>(column);
            const int         hexX      = m_hexX + column * 3 * m_charWidth;
            const int         asciiX    = m_asciiX + column * m_charWidth;

            if (byteIndex >= m_bytes.size())
            {
                continue;
            }

            const ByteColor color         = colorForByte(byteIndex);
            const bool      hexSelected   = isByteSelected(byteIndex, SelectionPart::Hex);
            const bool      asciiSelected = isByteSelected(byteIndex, SelectionPart::Ascii);
            const QRect     hexRect(hexX - 1, y + 1, 2 * m_charWidth + 2, m_lineHeight - 2);
            const QRect     asciiRect(asciiX - 1, y + 1, m_charWidth + 2, m_lineHeight - 2);
            painter.fillRect(hexRect, color.background);
            painter.fillRect(asciiRect, color.background);
            if (hexSelected)
            {
                painter.fillRect(hexRect.adjusted(0, 0, 0, 0), QColor(84, 110, 122, 90));
            }
            if (asciiSelected)
            {
                painter.fillRect(asciiRect.adjusted(0, 0, 0, 0), QColor(84, 110, 122, 90));
            }

            painter.setPen(color.foreground);
            const unsigned char byteValue = static_cast<unsigned char>(m_bytes[byteIndex]);
            painter.drawText(hexX, baseline, QString("%1").arg(static_cast<int>(byteValue), 2, 16, QLatin1Char('0')).toUpper());
            painter.drawText(asciiX, baseline, buildAsciiChar(byteValue));
        }
    }
}
