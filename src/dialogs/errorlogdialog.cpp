#include "errorlogdialog.h"
#include <QScrollBar>
#include "ui_errorlogdialog.h"

#include "errorlogmodel.h"
#include "errorlogfilter.h"

ErrorLogDialog::ErrorLogDialog(ErrorLogModel * pErrorLogModel, QWidget *parent) :
    QDialog(parent),
    _pUi(new Ui::ErrorLogDialog)
{
    _pUi->setupUi(this);

    _pErrorLogModel = pErrorLogModel;

    _pCategoryProxyFilter = new ErrorLogFilter();
    _pCategoryProxyFilter->setSourceModel(_pErrorLogModel);

    // Create button group for filtering
    _categoryFilterGroup.setExclusive(false);
    _categoryFilterGroup.addButton(_pUi->checkInfo);
    _categoryFilterGroup.addButton(_pUi->checkError);
    connect(&_categoryFilterGroup, static_cast<void (QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &ErrorLogDialog::handleFilterChange);
    this->handleFilterChange(0); // Update filter

    _pUi->listError->setModel(_pCategoryProxyFilter);
    _pUi->listError->setUniformItemSizes(true); // For performance
    _pUi->listError->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(_pUi->listError->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(handleScrollbarChange()));

    // Disable auto scroll when selecting an item
    connect(_pUi->listError->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(handleErrorSelectionChanged(QItemSelection,QItemSelection)));

    // Handle inserted row
    connect(_pUi->listError->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(handleLogsChanged()));
    connect(_pUi->listError->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(handleLogsChanged()));

    connect(_pUi->checkAutoScroll, SIGNAL(stateChanged(int)), this, SLOT(handleCheckAutoScrollChanged(int)));

    connect(_pUi->pushClear, SIGNAL(clicked(bool)), this, SLOT(handleClearButton()));

    // default to autoscroll
    setAutoScroll(true);
}

ErrorLogDialog::~ErrorLogDialog()
{
    delete _pUi;
}

void ErrorLogDialog::handleLogsChanged()
{
    updateScroll();

    updateLogCount();
}

void ErrorLogDialog::handleErrorSelectionChanged(QItemSelection selected, QItemSelection deselected)
{
    Q_UNUSED(deselected);

    /* Stop auto scroll when an item is selected */
    if (selected.size() != 0)
    {
        setAutoScroll(false);
    }
}

void ErrorLogDialog::handleCheckAutoScrollChanged(int newState)
{
    if (newState == Qt::Unchecked)
    {
        setAutoScroll(false);
    }
    else
    {
        setAutoScroll(true);
    }

}

void ErrorLogDialog::handleScrollbarChange()
{
    const QScrollBar * pScroll = _pUi->listError->verticalScrollBar();
    if (pScroll->value() == pScroll->maximum())
    {
        setAutoScroll(true);

        _pUi->listError->clearSelection();
    }
}

void ErrorLogDialog::handleClearButton()
{
    _pErrorLogModel->clear();
}

void ErrorLogDialog::handleFilterChange(int id)
{
    Q_UNUSED(id);

    quint32 bitmask = 0;

    if (_pUi->checkInfo->checkState())
    {
        bitmask |= 1 << ErrorLog::LOG_INFO;
    }

    if (_pUi->checkError->checkState())
    {
        bitmask |= 1 << ErrorLog::LOG_ERROR;
    }

    _pCategoryProxyFilter->setFilterBitmask(bitmask);

    updateLogCount();
}

void ErrorLogDialog::setAutoScroll(bool bAutoScroll)
{
    if (_bAutoScroll != bAutoScroll)
    {
        _bAutoScroll = bAutoScroll;

        _pUi->checkAutoScroll->setChecked(_bAutoScroll);

        updateScroll();
    }
}

void ErrorLogDialog::updateScroll()
{
    if (_bAutoScroll)
    {
        _pUi->listError->scrollToBottom();
    }
    else
    {
       // Don't auto scroll
    }
}

void ErrorLogDialog::updateLogCount()
{
    if (_pUi->checkInfo->checkState() == Qt::Checked || _pUi->checkError->checkState() == Qt::Checked)
    {
        _pUi->grpBoxLogs->setTitle(QString("Logs (%1/%2)").arg(_pCategoryProxyFilter->rowCount()).arg(_pErrorLogModel->size()));
    }
    else
    {
        _pUi->grpBoxLogs->setTitle(QString("Logs"));
    }
}
