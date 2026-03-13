#ifndef __HEX_VIEW_WIDGET_H__
#define __HEX_VIEW_WIDGET_H__

#include <QAbstractScrollArea>
#include <QColor>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

class HexViewWidget : public QAbstractScrollArea
{
public:
    explicit HexViewWidget(QWidget* parent = nullptr);

    void clear();
    void setBytes(const std::string& bytes);
    void setHighlightRange(std::size_t offset, std::size_t encodedLength, std::size_t valueLength);
    void scrollToOffset(std::size_t offset);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    enum class SelectionPart : std::uint8_t
    {
        None,
        Address,
        Hex,
        Ascii
    };

    struct HighlightRange
    {
        std::size_t offset;
        std::size_t encodedLength;
        std::size_t valueLength;
    };

    struct ByteColor
    {
        QColor background;
        QColor foreground;
    };

    struct SelectionRange
    {
        SelectionPart part  = SelectionPart::None;
        std::size_t   start = 0U;
        std::size_t   end   = 0U;

        bool isValid() const
        {
            return part != SelectionPart::None && start <= end;
        }
    };

private:
    void           copySelection() const;
    void           ensureVisibleRow(int row);
    void           updateMetrics();
    void           updateScrollBar();
    int            rowCount() const;
    int            visibleRowCount() const;
    ByteColor      colorForByte(std::size_t index) const;
    QString        buildAsciiChar(unsigned char value) const;
    int            rowFromY(int y) const;
    bool           isByteSelected(std::size_t index, SelectionPart part) const;
    bool           isRowSelected(int row) const;
    SelectionPart  selectionPartAt(int x) const;
    SelectionRange selectionRange() const;
    std::size_t    positionFromPoint(const QPoint& pos, SelectionPart part) const;
    QString        selectedText() const;
    void           drawHeader(QPainter& painter);
    void           drawRows(QPainter& painter);

private:
    std::string                   m_bytes;
    std::optional<HighlightRange> m_highlightRange;
    SelectionPart                 m_selectionPart;
    std::size_t                   m_selectionAnchor;
    std::size_t                   m_selectionCurrent;
    bool                          m_isSelecting;
    int                           m_charWidth;
    int                           m_lineHeight;
    int                           m_headerHeight;
    int                           m_addrX;
    int                           m_hexX;
    int                           m_asciiX;
    int                           m_totalWidth;
};

#endif
