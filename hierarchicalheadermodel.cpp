#include "hierarchicalheadermodel.h"
#include <QDebug>
#include <QStandardItem>

HierarchicalHeaderModel::HierarchicalHeaderModel(QStandardItemModel *model, QObject *parent) :
    QAbstractTableModel(parent),
    m_curSelectedIndex(QModelIndex()),
    m_curArrowIndex(QModelIndex()),
    m_headerModel(Q_NULLPTR)
{
    m_headerModel = model;
    if (m_headerModel != Q_NULLPTR) {
        m_headerModel->setParent(this);
        connect(m_headerModel, &QStandardItemModel::dataChanged, this, &HierarchicalHeaderModel::slotDataChanged);
        getHeaderList(m_headerList);

    }
}

HierarchicalHeaderModel::HierarchicalHeaderModel(const QStringList &headerList, QObject *parent) :
    QAbstractTableModel(parent),
    m_curSelectedIndex(QModelIndex()),
    m_curArrowIndex(QModelIndex()),
    m_headerModel(Q_NULLPTR)
{
    m_headerModel = new QStandardItemModel(this);
    connect(m_headerModel, &QStandardItemModel::dataChanged, this, &HierarchicalHeaderModel::slotDataChanged);
    int index = 0;
    for (int i = 0; i < headerList.count(); ++i) {
        QStandardItem *item = new QStandardItem(headerList.at(i));
        m_headerModel->setItem(0, index, item);
        ++index;
    }
    m_headerList = headerList;
}

HierarchicalHeaderModel::~HierarchicalHeaderModel()
{
    m_headerModel->deleteLater();
}

int HierarchicalHeaderModel::modelCount()
{
    if (m_headerModel != Q_NULLPTR)
        return m_headerModel->columnCount();

    return 0;
}

void HierarchicalHeaderModel::appendColumnItem(QStandardItem *item)
{
    if (item == Q_NULLPTR || m_headerModel == Q_NULLPTR)
        return;

    int column = m_headerModel->columnCount();
    beginInsertColumns(QModelIndex(), column, column + item->columnCount() - 1);
    m_headerModel->appendColumn({item});

    for (int i = 0; i < item->columnCount(); ++i) {
        m_headerList.append(item->child(0, i)->text());
    }
    endInsertColumns();
}

void HierarchicalHeaderModel::removeColumnItem(int preIndex, int startIndex, int endIndex)
{
    if (m_headerModel == Q_NULLPTR || preIndex < 0)
        return;

    int count = endIndex > 0 ? endIndex - startIndex + 1 : 1;
    if (count > m_headerList.count() - 2)
        return;

    QStandardItem *childItem = m_headerModel->item(0, preIndex);
    if (childItem == Q_NULLPTR)
        return;

    const int &childCount = childItem->columnCount();
    int listStart = 0;

    for (int i = 0; i < preIndex; ++i) {
        QStandardItem *item = m_headerModel->item(0, i);
        if (item->columnCount() == 0) {
            ++listStart;
        } else {
            listStart += item->columnCount();
        }
    }

    int start = listStart + (startIndex - 1) * childCount;
    int end = start + count * childCount;

    const int &prevSelectedColumn = getActualColumnIndex(m_curSelectedIndex);
    if (prevSelectedColumn >= start && prevSelectedColumn < end) {
        m_curSelectedIndex = QModelIndex();
    }

    beginResetModel();
//    beginRemoveColumns(QModelIndex(), start, start + count * childCount);
    m_headerModel->removeColumns(preIndex + startIndex - 1, count);
    m_headerList.erase(m_headerList.begin() + start,
        m_headerList.begin() + end);
    endResetModel();
//    endRemoveColumns();
}

/**
 * @brief HierarchicalHeaderModel::setColumnItemValue set title of top Item
 * @param column : top column number
 * @param value : name of title
 */
void HierarchicalHeaderModel::setColumnItemValue(int column, const QString &value)
{
    beginResetModel();

    if (m_headerModel == Q_NULLPTR)
        return;

    QStandardItem *item = m_headerModel->item(0, column);
    if (item == Q_NULLPTR)
        return;

    item->setText(value);

    if (column >= 0 && column < m_headerList.count()) {
        m_headerList[column] = value;
    }
    endResetModel();
}

void HierarchicalHeaderModel::setSectionTitle(int column, const QString &value, int sonColumn)
{
    if (column < 0 || column >= m_headerModel->columnCount())
        return;

    int index = 0;

    beginResetModel();
    QStandardItem *headerItem = m_headerModel->item(0, column);
    if (sonColumn < 0) {
        headerItem->setText(value);
        index = column;
    } else {
        if (headerItem == Q_NULLPTR ||
            sonColumn >= headerItem->columnCount())
            return;

        QStandardItem *sonItem = headerItem->child(0, sonColumn);
        if (sonItem != Q_NULLPTR) {
            sonItem->setText(value);
        }
        index = column + sonColumn;
    }

    if (index >= 0 && index < m_headerList.count()) {
        m_headerList[index] = value;
    }
    endResetModel();
}

QString HierarchicalHeaderModel::getSectionTitle(int column, int sonColumn)
{
    if (column < 0 || column >= m_headerModel->columnCount())
        return "";
    QStandardItem *headerItem = m_headerModel->item(0, column);
    if (sonColumn < 0) {
        return headerItem->text();
    } else {
        if (headerItem == Q_NULLPTR || sonColumn >= headerItem->columnCount())
            return "";
        QStandardItem *childItem = headerItem->child(0, sonColumn);
        if (childItem != Q_NULLPTR) {
            return childItem->text();
        }
    }

    return "";
}

int HierarchicalHeaderModel::getParentIndexByleafIndex(int leafIndex) const
{
    if (leafIndex < 0 ||
        leafIndex >= m_headerList.count() ||
        m_headerModel == Q_NULLPTR)
        return -1;

    int leafCount = 0;
    for (int i = 0; i < m_headerModel->columnCount(); ++i) {
        QStandardItem *headerItem = m_headerModel->item(0, i);
        int childCount = headerItem->columnCount();
        if (childCount == 0)
            childCount = 1;

        int tempCount = leafCount + childCount;
        if (leafIndex >= leafCount && leafIndex < tempCount)
            return i;
        leafCount = tempCount;
    }
    return -1;
}

int HierarchicalHeaderModel::getSelectedColumn() const
{
    return m_curSelectedIndex.isValid() ? getActualColumnIndex(m_curSelectedIndex) : -1;
}

int HierarchicalHeaderModel::getArrowIndex() const
{
    return m_curArrowIndex.isValid() ? getActualColumnIndex(m_curArrowIndex) : -1;
}

int HierarchicalHeaderModel::getArrowSortType() const
{
    QStandardItem *arrowItem = m_headerModel->itemFromIndex(m_curArrowIndex);
    if (arrowItem != Q_NULLPTR) {
        return arrowItem->data(ClickType::Arrow).toInt();
    }
    return 0;
}

int HierarchicalHeaderModel::getActualColumnIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return -1;

    const int &end = index.parent().isValid() ? index.parent().column() : index.column();
    int result = 0;
    for (int i = 0; i < end; ++i) {
        QStandardItem *headItem = m_headerModel->item(0, i);
        if (headItem != Q_NULLPTR) {
            result += headItem->columnCount() == 0 ? 1 : headItem->columnCount();
        }
    }
    if (index.parent().isValid()) {
        result += index.column();
    }
    return result;
}

int HierarchicalHeaderModel::rowCount(const QModelIndex &/*index*/) const
{
    return m_headerList.count();
}

int HierarchicalHeaderModel::columnCount(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return m_headerList.count();
}

QVariant HierarchicalHeaderModel::data(const QModelIndex &/*index*/, int role) const
{
    if (count() != 0)
    {
        if (role == HierarchicalHeaderView::HorizontalHeaderDataRole || role == HierarchicalHeaderView::VerticalHeaderDataRole) {
            QVariant v;
            v.setValue(m_headerModel);
            return v;
        }
    }
    return QVariant();
}

bool HierarchicalHeaderModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QAbstractTableModel::setData(index, value, role);
}

void HierarchicalHeaderModel::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles*/)
{
    if (m_headerModel == Q_NULLPTR)
        return;

    beginResetModel();
    const QVariant &selectData = m_headerModel->data(topLeft, selected);
    if (!selectData.isNull()) {

        if (selectData.toInt() > 0) {
            if (m_curSelectedIndex.isValid() && topLeft != m_curSelectedIndex) {
                QStandardItem *preItem = m_headerModel->itemFromIndex(m_curSelectedIndex);
                if (preItem != Q_NULLPTR) {
                    preItem->setData(QVariant(0), selected);
                }
            }
            m_curSelectedIndex = topLeft;
        }
    }

    const QVariant &arrowData = m_headerModel->data(topLeft, Arrow);
    if (!arrowData.isNull()) {

        if (arrowData.toInt() > 0) {
            if (m_curArrowIndex.isValid() && topLeft != m_curArrowIndex) {
                QStandardItem *preItem = m_headerModel->itemFromIndex(m_curArrowIndex);
                if (preItem != Q_NULLPTR) {
                    preItem->setData(QVariant(0), Arrow);
                }
            }
            m_curArrowIndex = topLeft;
        }
    }
    endResetModel();

}

void HierarchicalHeaderModel::getHeaderList(QStringList &str, QStandardItem *childItem) {

    if (m_headerModel == Q_NULLPTR)
        return;

    if (childItem == Q_NULLPTR) {
        for (int i = 0; i < m_headerModel->columnCount(); ++i) {
            QStandardItem *item = m_headerModel->item(0, i);
            if (item->hasChildren()) {
                getHeaderList(str, item);
            } else {
                str.append(item->text());
            }
        }
    } else {
        for (int i = 0; i < childItem->columnCount(); ++i) {
            QStandardItem *item = childItem->child(0, i);
            if (item->hasChildren()) {
                getHeaderList(str, item);
            } else {
                QString parentSuffix = QString("(%1)").arg(childItem->text());
                str.append(item->text() + parentSuffix);
            }
        }
    }
}
