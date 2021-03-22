#ifndef HIERARCHICALHEADERMODEL_H
#define HIERARCHICALHEADERMODEL_H
#include "hierarchicalheaderview.h"
#include <QAbstractTableModel>

class QStandardItem;
class QStandardItemModel;

class HierarchicalHeaderModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ClickType
    {
        selected = Qt::UserRole + 2,
        Arrow, /*= Qt::UserRole + 3*/ // 0 : noArrow , 1 : upArrow, 2 : downArrow
        FilterBtnState,
        CanFilter // 0 : disabled, 1 : filter(过滤), 2 : Filtered(已过滤)
    };
public:
    HierarchicalHeaderModel(QStandardItemModel *model, QObject *parent = 0);
    HierarchicalHeaderModel(const QStringList &headerList, QObject *parent = 0);
    virtual ~HierarchicalHeaderModel();

    inline int count() const { return m_headerList.count(); }
    int modelCount();

    inline QStringList headerList() const { return m_headerList; }
    void appendColumnItem(QStandardItem *item);
    void removeColumnItem(int preIndex, int startIndex, int endIndex = -1);

    void setColumnItemValue(int column, const QString &value);
    void setSectionTitle(int column, const QString &value, int sonColumn = -1);

    QString getSectionTitle(int column, int sonColumn = -1);
    int getParentIndexByleafIndex(int leafIndex) const;
    int getActualColumnIndex(const QModelIndex &index) const;

    int getSelectedColumn() const;
    int getArrowIndex() const;
    int getArrowSortType() const;

protected:
    int rowCount(const QModelIndex &index) const;
    int columnCount(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

private:
    void slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

private:
    void getHeaderList(QStringList &str, QStandardItem *childItem = Q_NULLPTR);

    QModelIndex m_curSelectedIndex;
    QModelIndex m_curArrowIndex;
    QStandardItemModel *m_headerModel;
    QStringList m_headerList;
};

#endif // HIERARCHICALHEADERMODEL_H
