#ifndef HIERARCHICAL_HEADER_VIEW_H
#define HIERARCHICAL_HEADER_VIEW_H

#include <QtWidgets/QHeaderView>
#include "hierarchicalheadermodel.h"

class HierarchicalHeaderView : public QHeaderView
{
    Q_OBJECT

public:

    enum HeaderDataModelRoles
    {
        HorizontalHeaderDataRole = Qt::UserRole,
        VerticalHeaderDataRole
    };

    enum ColorRole {
        SelectedBackGroundRole,
        UnSelectedBackGroundRole,
        BorderRole,
        TextRole
    };

    HierarchicalHeaderView(Qt::Orientation orientation, QWidget* parent = Q_NULLPTR);
    ~HierarchicalHeaderView();

    void setColor(ColorRole role, const QColor &color);

    void setModel(QAbstractItemModel* model);
    void setLeafAlignment(Qt::Alignment alignment);
    void setHeaderAlignment(Qt::Alignment alignment);

    void setSelectedColumn(const int &logicalIndex);
    bool getCanFilter();
    void setCanFilter(const bool &flag);
    bool getCanSort();
    void setCanSort(const bool &flag);

    void setColunmFilterState(const int &column, const int &value, const int &role);
    QModelIndex getModelIndexByColumn(const int &column);

    QSize sizeHint() const;

    inline void setFrozenHeader(HierarchicalHeaderView *header) { m_frozenHeader = header; }
    inline HierarchicalHeaderView *getFrozenHeader() const { return m_frozenHeader; }

signals:
    void signalArrowType(int column, bool Ascending);
    void signalFilterBtnClicked(const int &column, const QRect &popRect);

protected:
    void paintSection(QPainter* painter, const QRect &rect, int logicalIndex) const;
    QSize sectionSizeFromContents(int logicalIndex) const;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    bool viewportEvent(QEvent *e) override;

    bool checkIsFilterBtnClicked(const int &logicalIndex);
    void setClickSelectedColumn(int logicalIndex);
    int getPrevSelected() const;

    void setArrowColumn(const int &logicalIndex);
    void clearArrowType(int logicalIndex);

private slots:
    void slotSectionResized(int logicalIndex);
    void slotHeaderDataChange(int logicalIndex);

private:
    QStyleOptionHeader styleOptionForCell(int logicalIndex) const;

    class private_data;
    private_data *_pd;
    HierarchicalHeaderView *m_frozenHeader;

};

#endif
