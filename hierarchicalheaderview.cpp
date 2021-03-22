#include "hierarchicalheaderview.h"
#include <QPainter>
#include <QAbstractItemModel>
#include <QPointer>
#include <QVariant>
#include <QMouseEvent>

class HierarchicalHeaderView :: private_data
{
    Qt::Alignment m_leafAlignment;
    Qt::Alignment m_headerAlignment;
    bool m_canFilter;
    bool m_canSort;
    QVector<QColor> m_colors;

signals:
    void signalHeaderDataChange(int logicalIndex);

public:
    QPointer<QAbstractItemModel> headerModel;

    private_data() :
        m_leafAlignment(Qt::AlignCenter),
        m_headerAlignment(Qt::AlignCenter),
        m_canFilter(false),
        m_canSort(false)
    {
        m_colors = {
            QColor(255, 204, 153, 200),
            QColor(240, 240, 240, 200),
            QColor(210, 210, 210),
            QColor(0, 0, 0)
        };

    }

    HierarchicalHeaderModel *hierarchicalModel() const {
        if (headerModel.isNull())
            return Q_NULLPTR;

        return qobject_cast<HierarchicalHeaderModel*>(headerModel->QObject::parent());
    }

    void setColor(HierarchicalHeaderView::ColorRole role, const QColor &color) {
        if (role >= m_colors.size())
            return;

        m_colors[role] = color;
    }

    QColor getColor(HierarchicalHeaderView::ColorRole role) const {
        if (role >= m_colors.size())
            return QColor(255, 255, 255);

        return m_colors.at(role);
    }

    void setSelectedColumn(int column)
    {   
        if (headerModel.isNull() || column < 0)
            return;
        headerModel->setData(leafIndex(column), QVariant(1), HierarchicalHeaderModel::selected);
    }

    inline void clearSelection()
    {
        const QModelIndex &index = leafIndex(getPrevSelected());
        if (!headerModel.isNull() && index.isValid()) {
            headerModel->setData(index, QVariant(0), HierarchicalHeaderModel::selected);
        }
    }

    int setArrowType(int column)
    {
        if (headerModel.isNull() || column < 0)
            return -1;

        int value = headerModel->data(leafIndex(column), HierarchicalHeaderModel::Arrow).toInt();
        value = (value == 0) ? 1 : ((value == 1) ? 2 : 1);
        headerModel->setData(leafIndex(column), QVariant(value), HierarchicalHeaderModel::Arrow);
        return value;
    }

    inline void clearArrowType(int column) {
        if (headerModel.isNull() || column < -1)
            return;

        headerModel->setData(leafIndex(column), QVariant(0), HierarchicalHeaderModel::Arrow);
    }

    inline bool getCanFilter() { return m_canFilter; }
    inline void setCanFilter(const bool &flag) { m_canFilter = flag; }
    inline bool getCanSort() { return m_canSort; }
    inline void setCanSort(const bool &flag) { m_canSort = flag; }

    inline void setColunmFilterState(const int &column, const int &value, int role) {
        if (headerModel.isNull() || column < -1)
            return;
        headerModel->setData(leafIndex(column), QVariant(value), role);
    }

      int getPrevArrow() const {
          HierarchicalHeaderModel *model = hierarchicalModel();
          return model ? model->getArrowIndex() : -1;
      }

    int getPrevSelected() const {
        HierarchicalHeaderModel *model = hierarchicalModel();
        return model ? model->getSelectedColumn() : -1;
    }

    inline void setLeafAlignment(Qt::Alignment alignment)
    {
        m_leafAlignment = alignment;
    }

    inline void setHeaderAlignment(Qt::Alignment alignment)
    {
        m_headerAlignment = alignment;
    }

    void initFromNewModel(int orientation, QAbstractItemModel *model)
    {
        headerModel = QPointer<QAbstractItemModel>();
        QVariant v(model->data(
                       QModelIndex(),
                       (orientation == Qt::Horizontal ? HorizontalHeaderDataRole : VerticalHeaderDataRole)));

        if (v.isValid()) {
            headerModel = qobject_cast<QAbstractItemModel*>(v.value<QObject*>());
        }
    }

    QModelIndex findRootIndex(QModelIndex index) const
    {
        while (index.parent().isValid()) {
            index = index.parent();
        }
        return index;
    }

    QModelIndexList parentIndexes(QModelIndex index) const
    {
        QModelIndexList indexes;
        while (index.isValid())
        {
            indexes.push_front(index);
            index = index.parent();
        }
        return indexes;
    }

    QModelIndex findLeaf(const QModelIndex &curentIndex, int sectionIndex, int &curentLeafIndex)
    {
        if (curentIndex.isValid())
        {
            int childCount = curentIndex.model()->columnCount(curentIndex);
            if (childCount)
            {
                for (int i = 0; i < childCount; ++i)
                {
                    QModelIndex res(findLeaf(curentIndex.model()->index(0, i, curentIndex), sectionIndex, curentLeafIndex));
                    if (res.isValid())
                        return res;
                }
            }
            else
            {
                ++curentLeafIndex;
                if (curentLeafIndex == sectionIndex)
                    return curentIndex;
            }
        }
        return QModelIndex();
    }

    QModelIndex leafIndex(int sectionIndex)
    {
        if (!headerModel.isNull())
        {
            int curentLeafIndex = -1;
            for (int i = 0; i < headerModel->columnCount(); ++i)
            {
                QModelIndex res(findLeaf(headerModel->index(0, i), sectionIndex, curentLeafIndex));
                if (res.isValid())
                    return res;
            }
        }
        return QModelIndex();
    }

    QModelIndexList searchLeafs(const QModelIndex& curentIndex) const
    {
        QModelIndexList res;
        if (curentIndex.isValid())
        {
            int childCount=curentIndex.model()->columnCount(curentIndex);
            if (childCount)
            {
                for (int i = 0; i < childCount; ++i)
                    res += searchLeafs(curentIndex.model()->index(0, i, curentIndex));
            }
            else
            {
                res.push_back(curentIndex);
            }
        }
        return res;
    }

    QModelIndexList leafs(const QModelIndex &searchedIndex) const
    {
        QModelIndexList leafs;
        if (searchedIndex.isValid())
        {
            int childCount = searchedIndex.model()->columnCount(searchedIndex);
            for (int i = 0; i < childCount; ++i)
                leafs += searchLeafs(searchedIndex.model()->index(0, i, searchedIndex));
        }
        return leafs;
    }

    void setForegroundBrush(QStyleOptionHeader &opt, const QModelIndex &index) const
    {
        QVariant foregroundBrush = index.data(Qt::ForegroundRole);
        if (foregroundBrush.canConvert(QMetaType::QBrush))
            opt.palette.setBrush(QPalette::ButtonText, qvariant_cast<QBrush>(foregroundBrush));
    }

    void setBackgroundBrush(QStyleOptionHeader &opt, const QModelIndex &index) const
    {
        QVariant backgroundBrush = index.data(Qt::BackgroundRole);
        if (backgroundBrush.canConvert(QMetaType::QBrush))
        {
            opt.palette.setBrush(QPalette::Button, qvariant_cast<QBrush>(backgroundBrush));
            opt.palette.setBrush(QPalette::Window, qvariant_cast<QBrush>(backgroundBrush));
        }
    }

    QSize cellSize(const QModelIndex& leafIndex, const QHeaderView* hv, QStyleOptionHeader styleOptions) const
    {
        QSize res;
        QVariant variant(leafIndex.data(Qt::SizeHintRole));
        if (variant.isValid())
            res = qvariant_cast<QSize>(variant);
        QFont fnt(hv->font());
        QVariant var(leafIndex.data(Qt::FontRole));
        if (var.isValid() && var.canConvert(QMetaType::QFont))
            fnt = qvariant_cast<QFont>(var);
        fnt.setBold(true);
        QFontMetrics fm(fnt);
        QSize size(fm.size(0, leafIndex.data(Qt::DisplayRole).toString()));
        if (leafIndex.data(Qt::UserRole).isValid())
            size.transpose();
        QSize decorationsSize(hv->style()->sizeFromContents(QStyle::CT_HeaderSection, &styleOptions, QSize(), hv));
        QSize emptyTextSize(fm.size(0, ""));
        return res.expandedTo(size + decorationsSize - emptyTextSize);
    }

    int	currentCellWidth(const QModelIndex &searchedIndex, const QModelIndex &leafIndex,
                         int sectionIndex, const QHeaderView *hv) const
    {
        QModelIndexList leafsList(leafs(searchedIndex));
        if (leafsList.empty())
            return hv->sectionSize(sectionIndex);
        int width = 0;
        int firstLeafSectionIndex = sectionIndex-leafsList.indexOf(leafIndex);
        for (int i = 0; i < leafsList.size(); ++i)
            width += hv->sectionSize(firstLeafSectionIndex+i);
        return width;
    }

    int	currentCellLeft(const QModelIndex &searchedIndex, const QModelIndex &leafIndex,
                        int sectionIndex, int left, const QHeaderView *hv) const
    {
        QModelIndexList leafsList(leafs(searchedIndex));
        if (!leafsList.empty())
        {
            int n = leafsList.indexOf(leafIndex);
            int firstLeafSectionIndex = sectionIndex - n;
            --n;
            for (; n >= 0; --n)
                left -= hv->sectionSize(firstLeafSectionIndex+n);
        }
        return left;
    }

    void paintCell(QPainter *painter, const QModelIndex &cellIndex,
                   const QModelIndex &leafIndex, const QHeaderView* hv,
                   QStyleOptionHeader& styleOptions) const
    {

        if (!painter) return;

        painter->save();

        const QRect &rect = styleOptions.rect;
        if (cellIndex.parent().isValid() || cellIndex == leafIndex) {
            const QRect &rect = styleOptions.rect;
            painter->fillRect(rect, styleOptions.palette.window());
            int type = headerModel->data(leafIndex, HierarchicalHeaderModel::Arrow).toInt();
            if (type > 0) {
                painter->save();
                QStyleOptionHeader opt(styleOptions);

                int triangleW = 15;
                int triLeft = rect.left() + ((rect.width() - triangleW) >> 1);
                QRect triangle(triLeft, rect.top(), triangleW, (triangleW >> 1));

                QStyle::PrimitiveElement pe = (type == 1) ? QStyle::PE_IndicatorArrowDown : QStyle::PE_IndicatorArrowUp;
                opt.rect = triangle;
                opt.palette.setBrush(QPalette::ButtonText, QBrush(QColor(73, 179, 238)));
                hv->style()->drawPrimitive(pe, &opt, painter, hv);
                painter->restore();
            }
        }

        if (cellIndex != leafIndex)
            painter->eraseRect(rect);

        painter->setPen(getColor(HierarchicalHeaderView::TextRole));
        painter->drawText(rect, styleOptions.text, QTextOption(styleOptions.textAlignment));
        painter->setPen(getColor(HierarchicalHeaderView::BorderRole));
        const QRect &newRect = rect.adjusted(-1, -1, -1, -1);
        painter->drawRect(newRect);

        painter->restore();
    }

    int paintHorizontalCell(QPainter *painter, const QHeaderView *hv, const QModelIndex &cellIndex,
                            const QModelIndex &leafIndex, int logicalLeafIndex,
                            const QStyleOptionHeader &styleOptions, const QRect &sectionRect, int top) const
    {
        QStyleOptionHeader uniopt(styleOptions);

        const QVariant &variant = headerModel->data(leafIndex, HierarchicalHeaderModel::selected);

        QColor color;
        if (variant.isValid() && variant.toInt() == 1)
            color = getColor(HierarchicalHeaderView::SelectedBackGroundRole);
        else
            color = getColor(HierarchicalHeaderView::UnSelectedBackGroundRole);
        QBrush brush(color);
        uniopt.palette.setBrush(QPalette::Window, brush);

        int height = cellSize(cellIndex, hv, uniopt).height();
        if (cellIndex == leafIndex) {
            uniopt.textAlignment = m_headerAlignment;
            height = sectionRect.height() - top;
        } else {
            uniopt.textAlignment = m_leafAlignment;
        }
        int left = currentCellLeft(cellIndex, leafIndex, logicalLeafIndex, sectionRect.left(), hv);
        int width = currentCellWidth(cellIndex, leafIndex, logicalLeafIndex, hv);

        uniopt.text = cellIndex.data(Qt::DisplayRole).toString();
        uniopt.rect = QRect(left, top, width, height);

        paintCell(painter, cellIndex, leafIndex, hv, uniopt);
//        if (cellIndex.parent().isValid() || cellIndex == leafIndex) {
//            painter->fillRect(uniopt.rect, uniopt.palette.window());
//            int type = headerModel->data(leafIndex, HierarchicalHeaderModel::Arrow).toInt();
//            if (type > 0) {
//                painter->save();
//                QStyleOptionHeader opt(uniopt);

//                int triangleW = 15;
//                int triLeft = left + ((width - triangleW) >> 1);
//                QRect triangle(triLeft, top, triangleW, (triangleW >> 1));

//                QStyle::PrimitiveElement pe = (type == 1) ? QStyle::PE_IndicatorArrowDown : QStyle::PE_IndicatorArrowUp;
//                opt.rect = triangle;
//                opt.palette.setBrush(QPalette::ButtonText, QBrush(QColor(73, 179, 238)));
//                hv->style()->drawPrimitive(pe, &opt, painter, hv);
//                painter->restore();
//            }
//        }

//        if (cellIndex != leafIndex)
//            painter->eraseRect(uniopt.rect);

//        painter->drawText(uniopt.rect, uniopt.text, QTextOption(uniopt.textAlignment));
//        painter->setPen(QColor(210, 210, 210));
//        const QRect &newRect = uniopt.rect.adjusted(-1, -1, -1, -1);
//        painter->drawRect(newRect);
//        painter->restore();
        if (cellIndex == leafIndex && m_canFilter)
            paintFilterCell(painter,leafIndex, left, width, height, top);
        return top + height;
    }

    void paintFilterCell(QPainter *painter, const QModelIndex &leafIndex, const int &left,
                         const int &width, const int &height, const int &top) const
    {
        int type = headerModel->data(leafIndex, HierarchicalHeaderModel::CanFilter).toInt();
        if (type == 0)
            return;
        painter->save();
        // draw 口
        int frameSize = 16;
        int frameTop = top + height - frameSize - 2;
        int frameLeft = left + width - frameSize - 2;

        QBrush curBrush(Qt::black);
        QPen curPen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin);
        QRect curRect(frameLeft, frameTop, frameSize, frameSize);
        curPen.setColor(QColor(150, 153, 163));
        curPen.setWidth(1);
        curBrush.setColor(QColor(236, 237, 239));
        painter->setBrush(curBrush);
        painter->setPen(curPen);
        painter->drawRect(curRect);

//        QPixmap pixmap;
//        QString iconPath = ":image/downBtn.png";
//        if (type == 2) {
//           iconPath = ":image/toolBar/filter.png";
//        }
//        pixmap.load(iconPath);
//        painter->drawPixmap(frameLeft +1, frameTop +1, frameSize, frameSize, pixmap);

        //draw ▼
        int triangleWidth = 8;
        int triangLength = 6;
        int triangTop = frameTop + (frameSize - triangLength) / 2;
        int triangLeft = frameLeft + (frameSize - triangleWidth) / 2;
        QPoint points[3] = {QPoint(triangLeft, triangTop),
                            QPoint(triangLeft + triangleWidth, triangTop),
                            QPoint(triangLeft + triangleWidth /2, triangTop + triangLength)};

        curBrush.setColor(QColor(90, 90, 102));
        painter->setPen(curPen);
        painter->setBrush(curBrush);
        painter->drawConvexPolygon(points, 3);
        painter->restore();
    }

    void paintHorizontalSection(QPainter *painter, const QRect &sectionRect, int logicalLeafIndex,
                                const QHeaderView *hv, const QStyleOptionHeader &styleOptions,
                                const QModelIndex &leafIndex) const
    {
        QPointF oldBO(painter->brushOrigin());
        int top = sectionRect.y();
        QModelIndexList indexes(parentIndexes(leafIndex));
        for (int i = 0; i < indexes.size(); ++i)
        {
            QStyleOptionHeader realStyleOptions(styleOptions);

            top = paintHorizontalCell(painter,
                                    hv,
                                    indexes[i],
                                    leafIndex,
                                    logicalLeafIndex,
                                    realStyleOptions,
                                    sectionRect,
                                    top);
        }
        painter->setBrushOrigin(oldBO);
    }

    int paintVerticalCell(QPainter *painter, const QHeaderView *hv, const QModelIndex &cellIndex,
                          const QModelIndex &leafIndex, int logicalLeafIndex,
                          const QStyleOptionHeader &styleOptions, const QRect &sectionRect, int left) const
    {
        QStyleOptionHeader uniopt(styleOptions);

        const QVariant &variant = headerModel->data(leafIndex, HierarchicalHeaderModel::selected);
        QColor color;
        if (variant.isValid() && variant.toInt() == 1)
            color = getColor(HierarchicalHeaderView::SelectedBackGroundRole);
        else
            color = getColor(HierarchicalHeaderView::UnSelectedBackGroundRole);

        QBrush brush(color);
        uniopt.palette.setBrush(QPalette::Window, brush);

        int width = cellSize(cellIndex, hv, uniopt).width() + 2;
        if (cellIndex == leafIndex)
            width = sectionRect.width() - left;

        int top = currentCellLeft(cellIndex, leafIndex, logicalLeafIndex, sectionRect.top(), hv);
        int height = currentCellWidth(cellIndex, leafIndex, logicalLeafIndex, hv);

        QRect r(left, top, width, height);

        uniopt.text = cellIndex.data(Qt::DisplayRole).toString();
        uniopt.rect = r;

//        painter->save();

//        if (cellIndex.data(Qt::UserRole).isValid() && 0)
//        {
//            hv->style()->drawControl(QStyle::CE_HeaderSection, &uniopt, painter, hv);
//            QMatrix m;
//            m.rotate(-90);
//            painter->setWorldTransform(QTransform(m), true);

//            QRect new_r(0, 0,  r.height(), r.width());
//            new_r.moveCenter(QPoint(-r.center().y(), r.center().x()));
//            uniopt.rect = new_r;
//            hv->style()->drawControl(QStyle::CE_HeaderLabel, &uniopt, painter, hv);
//        }
//        else
//        {
//            hv->style()->drawControl(QStyle::CE_Header, &uniopt, painter, hv);
//        }

//        if (cellIndex.parent().isValid() || cellIndex == leafIndex) {
//            painter->fillRect(uniopt.rect, uniopt.palette.window());
//            int type = headerModel->data(leafIndex, HierarchicalHeaderModel::Arrow).toInt();
//            if (type > 0) {
//                painter->save();
//                QStyleOptionHeader opt(uniopt);

//                int triangleW = 15;
//                int triLeft = left + ((width - triangleW) >> 1);
//                QRect triangle(triLeft, top, triangleW, (triangleW >> 1));

//                QStyle::PrimitiveElement pe = (type == 1) ? QStyle::PE_IndicatorArrowDown : QStyle::PE_IndicatorArrowUp;
//                opt.rect = triangle;
//                opt.palette.setBrush(QPalette::ButtonText, QBrush(QColor(73, 179, 238)));
//                hv->style()->drawPrimitive(pe, &opt, painter, hv);
//                painter->restore();
//            }
//        }

//        if (cellIndex != leafIndex)
//            painter->eraseRect(uniopt.rect);

//        painter->drawText(uniopt.rect, uniopt.text, QTextOption(uniopt.textAlignment));
//        painter->setPen(QColor(210, 210, 210));
//        const QRect &newRect = uniopt.rect.adjusted(-1, -1, -1, -1);
//        painter->drawRect(newRect);
        paintCell(painter, cellIndex, leafIndex, hv, uniopt);

        return left + width;
    }

    void paintVerticalSection(QPainter *painter, const QRect& sectionRect, int logicalLeafIndex,
                              const QHeaderView* hv, const QStyleOptionHeader& styleOptions,
                              const QModelIndex& leafIndex) const
    {
        QPointF oldBO(painter->brushOrigin());
        int left = sectionRect.x();
        QModelIndexList indexes(parentIndexes(leafIndex));
        for (int i = 0; i < indexes.size(); ++i)
        {
            QStyleOptionHeader realStyleOptions(styleOptions);

            left = paintVerticalCell(painter,
                                   hv,
                                   indexes[i],
                                   leafIndex,
                                   logicalLeafIndex,
                                   realStyleOptions,
                                   sectionRect,
                                   left);
        }
        painter->setBrushOrigin(oldBO);
    }
};

HierarchicalHeaderView::HierarchicalHeaderView(Qt::Orientation orientation, QWidget *parent) :
    QHeaderView(orientation, parent),
    _pd(new private_data()),
    m_frozenHeader(Q_NULLPTR)
{
    setStyleSheet("background-color:rgb(240, 240, 240);border-color:rgb(210,210,210);");
    setHighlightSections(true);
    connect(this, SIGNAL(sectionResized(int, int, int)), this, SLOT(slotSectionResized(int)));
}

HierarchicalHeaderView::~HierarchicalHeaderView()
{
    delete _pd;
    _pd = Q_NULLPTR;
}

void HierarchicalHeaderView::setColor(HierarchicalHeaderView::ColorRole role, const QColor &color)
{
    if (_pd) _pd->setColor(role, color);
    viewport()->update();
}

QStyleOptionHeader HierarchicalHeaderView::styleOptionForCell(int logicalInd) const
{
    QStyleOptionHeader opt;
    initStyleOption(&opt);
    if (window()->isActiveWindow())
        opt.state |= QStyle::State_Active;
    opt.textAlignment = Qt::AlignCenter;
    opt.iconAlignment = Qt::AlignVCenter;
    opt.section = logicalInd;

    int visual = visualIndex(logicalInd);

    if (count() == 1)
    {
        opt.position = QStyleOptionHeader::OnlyOneSection;
    }
    else
    {
        if (visual == 0)
            opt.position = QStyleOptionHeader::Beginning;
        else
            opt.position=(visual==count()-1 ? QStyleOptionHeader::End : QStyleOptionHeader::Middle);
    }

    if (sectionsClickable())
    {
        if (highlightSections() && selectionModel())
        {
            if (orientation()==Qt::Horizontal)
            {
                if (selectionModel()->columnIntersectsSelection(logicalInd, rootIndex()))
                    opt.state |= QStyle::State_On;
                if (selectionModel()->isColumnSelected(logicalInd, rootIndex()))
                    opt.state |= QStyle::State_Sunken;
            }
        }
    }

    if (selectionModel())
    {
        bool previousSelected=false;
        if (orientation() == Qt::Horizontal)
            previousSelected = selectionModel()->isColumnSelected(logicalIndex(visual - 1), rootIndex());
        else
            previousSelected = selectionModel()->isRowSelected(logicalIndex(visual - 1), rootIndex());
        bool nextSelected=false;
        if (orientation() == Qt::Horizontal)
            nextSelected = selectionModel()->isColumnSelected(logicalIndex(visual + 1), rootIndex());
        else
            nextSelected = selectionModel()->isRowSelected(logicalIndex(visual + 1), rootIndex());
        if (previousSelected && nextSelected)
            opt.selectedPosition = QStyleOptionHeader::NextAndPreviousAreSelected;
        else
        {
            if (previousSelected)
                opt.selectedPosition = QStyleOptionHeader::PreviousIsSelected;
            else
            {
                if (nextSelected)
                    opt.selectedPosition = QStyleOptionHeader::NextIsSelected;
                else
                    opt.selectedPosition = QStyleOptionHeader::NotAdjacent;
            }
        }
    }
    return opt;
}

QSize HierarchicalHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    if (_pd->headerModel)
    {
        QModelIndex curLeafIndex(_pd->leafIndex(logicalIndex));
        if (curLeafIndex.isValid()/* && !isSectionHidden(logicalIndex)*/)
        {
            QStyleOptionHeader styleOption(styleOptionForCell(logicalIndex));
            QSize s(_pd->cellSize(curLeafIndex, this, styleOption));
            curLeafIndex = curLeafIndex.parent();
            while (curLeafIndex.isValid())
            {
                if (orientation() == Qt::Horizontal)
                    s.rheight() += _pd->cellSize(curLeafIndex, this, styleOption).height();
                else
                    s.rwidth() += _pd->cellSize(curLeafIndex, this, styleOption).width();
                curLeafIndex = curLeafIndex.parent();
            }
            return s;
        }
    }
    return QHeaderView::sectionSizeFromContents(logicalIndex);
}

void HierarchicalHeaderView::mousePressEvent(QMouseEvent *e)
{
    if (cursor().shape() != Qt::SplitHCursor && sectionsClickable() && e->button() == Qt::LeftButton) {
        int logicalIndex = logicalIndexAt(e->pos());
        if (logicalIndex < 0)
            return;
        if (checkIsFilterBtnClicked(logicalIndex)) {
        }
        else if (getCanSort())
        {
            if (e->modifiers() == Qt::ControlModifier) {
                clearArrowType(logicalIndex);
            }
            else
                setClickSelectedColumn(logicalIndex);
        }
        setSelectedColumn(logicalIndex);
    }
    return QHeaderView::mousePressEvent(e);
}

void HierarchicalHeaderView::mouseMoveEvent(QMouseEvent *e)
{
    return QHeaderView::mouseMoveEvent(e);
}

void HierarchicalHeaderView::mouseReleaseEvent(QMouseEvent *e)
{
    return QHeaderView::mouseReleaseEvent(e);
}

bool HierarchicalHeaderView::viewportEvent(QEvent *e)
{
    return QHeaderView::viewportEvent(e);
}

bool HierarchicalHeaderView::checkIsFilterBtnClicked(const int &logicalIndex)
{
    QModelIndex leafIndex(_pd->leafIndex(logicalIndex));
    if (!leafIndex.isValid() || !getCanFilter())
        return false;
    int type = !_pd->headerModel.isNull() ? _pd->headerModel->data(leafIndex, HierarchicalHeaderModel::CanFilter).toInt() : 0;
    if (type == 0)
        return false;
    int frameSize = 14;
    QPoint pt = this->mapFromGlobal(QCursor::pos());
    int columnleft = sectionViewportPosition(logicalIndex);
    int width = _pd->currentCellWidth(leafIndex, leafIndex, logicalIndex, this);
    int height = viewport()->height();
    int frameLeft = columnleft + width - frameSize - 2;
    if ((frameLeft <= pt.x() && (pt.x() <= columnleft + width))
            && (pt.y() < height && (pt.y() > height - frameSize - 2))) {
        QRect popRect(columnleft, 0, width, height);
        bool btnState = !_pd->headerModel.isNull() ? _pd->headerModel->data(leafIndex, HierarchicalHeaderModel::FilterBtnState).toBool() : false;
        if (!btnState) {
            emit signalFilterBtnClicked(logicalIndex, popRect);
        }
        int colunm = getPrevSelected();
        setColunmFilterState(logicalIndex, !btnState, HierarchicalHeaderModel::FilterBtnState);
        if (colunm != logicalIndex)
            setColunmFilterState(colunm, false, HierarchicalHeaderModel::FilterBtnState);
        return  true;
    }
    return  false;
}

void HierarchicalHeaderView::setSelectedColumn(const int &logicalIndex)
{
    if (logicalIndex < 0) {
        _pd->clearSelection();
        int select = _pd->getPrevSelected();
        headerDataChanged(Qt::Horizontal, select, select);
    } else {
        int column = _pd->getPrevSelected();
        if (column != logicalIndex) {
            _pd->setSelectedColumn(logicalIndex);
            headerDataChanged(Qt::Horizontal, column, column);
        }
    }
}

void HierarchicalHeaderView::setArrowColumn(const int &logicalIndex)
{
    int column = _pd->getPrevArrow();
    int value = _pd->setArrowType(logicalIndex);
    headerDataChanged(Qt::Horizontal, column, column);

    if (value != 0) {
        bool ascending = (value == 1);
        emit signalArrowType(logicalIndex, ascending);
    }
}

bool HierarchicalHeaderView::getCanFilter()
{
    return _pd->getCanFilter();
}

void HierarchicalHeaderView::setCanFilter(const bool &flag)
{
    _pd->setCanFilter(flag);
}

bool HierarchicalHeaderView::getCanSort()
{
    return _pd->getCanSort();
}

void HierarchicalHeaderView::setCanSort(const bool &flag)
{
    _pd->setCanSort(flag);
}

void HierarchicalHeaderView::setColunmFilterState(const int &column, const int &value, const int &role)
{
    _pd->setColunmFilterState(column, value, role);
}

void HierarchicalHeaderView::clearArrowType(int logicalIndex)
{
    _pd->clearArrowType(logicalIndex);
}

QModelIndex HierarchicalHeaderView::getModelIndexByColumn(const int &column)
{
    return _pd->leafIndex(column);
}

QSize HierarchicalHeaderView::sizeHint() const
{
    QSize newSize = QHeaderView::sizeHint();
    if (m_frozenHeader != Q_NULLPTR) {
        const QSize &frozenSize = m_frozenHeader->QHeaderView::sizeHint();
        if (frozenSize.height() > newSize.height())
            newSize.setHeight(frozenSize.height());
    }

    return newSize;
}

void HierarchicalHeaderView::setClickSelectedColumn(int logicalIndex)
{
    setArrowColumn(logicalIndex);
//    setSelectedColumn(logicalIndex);
}

int HierarchicalHeaderView::getPrevSelected() const
{
    return _pd->getPrevSelected();
}

void HierarchicalHeaderView::paintSection(QPainter *painter,
                const QRect &rect, int logicalIndex) const
{
    if (rect.isValid())
    {
        QModelIndex leafIndex(_pd->leafIndex(logicalIndex));
        if (leafIndex.isValid())
        {
            if (orientation() == Qt::Horizontal)
                _pd->paintHorizontalSection(painter, rect, logicalIndex, this, styleOptionForCell(logicalIndex), leafIndex);
            else
                _pd->paintVerticalSection(painter, rect, logicalIndex, this, styleOptionForCell(logicalIndex), leafIndex);
            return;
        }
    }
    return QHeaderView::paintSection(painter, rect, logicalIndex);
}

void HierarchicalHeaderView::slotSectionResized(int logicalIndex)
{
    if (isSectionHidden(logicalIndex))
        return;

    QModelIndex leafIndex(_pd->leafIndex(logicalIndex));
    if (leafIndex.isValid())
    {
        QModelIndexList leafsList(_pd->leafs(_pd->findRootIndex(leafIndex)));
        for (int n=leafsList.indexOf(leafIndex); n>0; --n)
        {
            --logicalIndex;

            int w = viewport()->width();
            int h = viewport()->height();
            int pos = sectionViewportPosition(logicalIndex);
            QRect r(pos, 0, w - pos, h);
            if (orientation() == Qt::Horizontal)
            {
                if (isRightToLeft())
                    r.setRect(0, 0, pos + sectionSize(logicalIndex), h);
            }
            else
                r.setRect(0, pos, w, h - pos);

            viewport()->update(r.normalized());
        }
    }
}

void HierarchicalHeaderView::slotHeaderDataChange(int logicalIndex)
{
    headerDataChanged(Qt::Horizontal, logicalIndex, logicalIndex);
}

void HierarchicalHeaderView::setModel(QAbstractItemModel *model)
{
    _pd->initFromNewModel(orientation(), model);
    QHeaderView::setModel(model);
    int cnt = (orientation() == Qt::Horizontal ? model->columnCount() : model->rowCount());
    if (cnt) initializeSections(0, cnt - 1);
}

void HierarchicalHeaderView::setLeafAlignment(Qt::Alignment alignment)
{
    _pd->setLeafAlignment(alignment);
}

void HierarchicalHeaderView::setHeaderAlignment(Qt::Alignment alignment)
{
    _pd->setHeaderAlignment(alignment);
}

